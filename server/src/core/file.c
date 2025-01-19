#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <errno.h>
#include <zip.h>
#include <dirent.h>
#include "../../include/core/file.h"
#include "../../include/database.h"
#include "../../include/config.h" 
#include "../../include/utils/jwt_utils.h"
#include "../../include/utils/checkInput_utils.h"
#include "../../include/utils/macro_utils.h"

#define BUFFER_SIZE 1024  // Kích thước buffer 1KB

//===============================================================================================================================
void receive_file(int sockfd, const char *data) {
    CHECK_DB_CONN(sockfd, "SEND_FILE", conn);

    // Tách giá trị từ dữ liệu đầu vào
    char *parsed_values[3];
    VALIDATE_INPUT(data, receive_file_config, parsed_values, 3, "SEND_FILE", sockfd, conn);
    char *file_name = parsed_values[0];
    int num_chunks = atoi(parsed_values[1]);  // Số lượng chunk
    char *token = parsed_values[2];

    // Xác thực người dùng qua token
    int user_id = verify_jwt(token, config->SECRET_KEY, conn);
    if (user_id == -1) {
        SEND_ERROR(sockfd, "SEND_FILE", 401, "Unauthorized", conn, NULL);
        free_parsed_values(parsed_values, 3);
        return;
    }

    // Cập nhật đường dẫn tệp
    char output_file[BUFFER_SIZE];
    if (snprintf(output_file, sizeof(output_file), "uploads/%d/%s", user_id, file_name) >= (int)sizeof(output_file)) {
        fprintf(stderr, "File path too long\n");
        SEND_ERROR(sockfd, "SEND_FILE", 400, "Bad Request", conn, NULL);
        free_parsed_values(parsed_values, 3);
        return;
    }

    // Đảm bảo thư mục tồn tại
    char dir_path[BUFFER_SIZE];
    snprintf(dir_path, sizeof(dir_path), "uploads/%d", user_id);
    if (mkdir(dir_path, 0755) == -1 && errno != EEXIST) {
        SEND_ERROR(sockfd, "SEND_FILE", 500, "Server Error", conn, NULL);
        free_parsed_values(parsed_values, 3);
        return;
    }

    // Kiểm tra nếu file đã tồn tại và tạo tên mới (index) nếu cần
    struct stat statbuf;
    if (stat(output_file, &statbuf) == 0) {
        char temp_file_name[BUFFER_SIZE], file_ext[128];
        char *dot_pos = strrchr(file_name, '.');

        if (dot_pos != NULL) {
            size_t name_len = dot_pos - file_name;
            strncpy(temp_file_name, file_name, name_len);
            temp_file_name[name_len] = '\0';
            strncpy(file_ext, dot_pos, sizeof(file_ext) - 1);
        } else {
            strncpy(temp_file_name, file_name, sizeof(temp_file_name) - 1);
            file_ext[0] = '\0';
        }

        int index = 1;
        char new_file_path[BUFFER_SIZE];
        while (1) {
            int written = snprintf(new_file_path, sizeof(new_file_path), "uploads/%d/%s(%d)%s", user_id, temp_file_name, index, file_ext);
            if (written >= (int)sizeof(new_file_path)) {
                fprintf(stderr, "Tên file quá dài\n");
                SEND_ERROR(sockfd, "SEND_FILE", 400, "Bad Request", conn, NULL);
                free_parsed_values(parsed_values, 3);
                return;
            }

            if (stat(new_file_path, &statbuf) != 0) {
                strcpy(output_file, new_file_path);
                break;
            }
            index++;
        }
    }

    // Mở file
    FILE *file = fopen(output_file, "wb");
    if (file == NULL) {
        SEND_ERROR(sockfd, "SEND_FILE", 500, "Failed to create file", conn, NULL);
        free_parsed_values(parsed_values, 3);
        return;
    }

    // Lấy kích thước của tệp
    char buffer[BUFFER_SIZE];
    ssize_t bytes_received;
    int chunks_received = 0;

    // Gửi ACK cho client ngay sau khi nhận yêu cầu
    if (send(sockfd, "ACK", 3, 0) < 0) {
        perror("Lỗi khi gửi tín hiệu ACK");
        SEND_ERROR(sockfd, "SEND_FILE", 500, "Error sending ACK", conn, NULL);
        fclose(file);
        free_parsed_values(parsed_values, 3);
        return;
    }

    // Nhận và ghi các chunk vào tệp
    while (chunks_received < num_chunks) {
        bytes_received = recv(sockfd, buffer, sizeof(buffer), 0);
        if (bytes_received <= 0) {
            break;
        }

        size_t bytes_written = fwrite(buffer, 1, bytes_received, file);
        if (bytes_written != (size_t)bytes_received) {
            perror("Lỗi khi ghi dữ liệu vào file");
            break;
        }

        chunks_received++;

        // Gửi ACK sau mỗi chunk nhận được
        if (send(sockfd, "ACK", 3, 0) < 0) {
            perror("Error sending ACK");
            break;
        }
    }

    fclose(file);

    //Gửi phản hồi thành công cho client
    char success_message[BUFFER_SIZE];
    snprintf(success_message, sizeof(success_message), "SEND_FILE|SUCCESS|200|%s", file_name);
    success_message[sizeof(success_message) - 1] = '\0';
    if (send(sockfd, success_message, strlen(success_message), 0) < 0) {
        perror("Error sending success message");
    }

    disconnect_db(conn);
    free_parsed_values(parsed_values, 3);
}

//===============================================================================================================================
int zip_directory(const char *folder_path, const char *zip_file_path) {
    int err = 0;

    // Mở file ZIP để ghi dữ liệu
    zip_t *zip = zip_open(zip_file_path, ZIP_CREATE | ZIP_EXCL, &err);
    if (!zip) {
        fprintf(stderr, "Error opening zip file %s: %d\n", zip_file_path, err);
        return -1;
    }

    DIR *dir = opendir(folder_path);
    if (dir == NULL) {
        perror("Failed to open directory");
        zip_close(zip);
        return -1;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        // Bỏ qua các thư mục con "." và ".."
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        char full_path[512];
        snprintf(full_path, sizeof(full_path), "%s/%s", folder_path, entry->d_name);

        struct stat statbuf;
        if (stat(full_path, &statbuf) == 0 && S_ISDIR(statbuf.st_mode)) {
            // Nếu là thư mục, tiếp tục nén đệ quy
            zip_directory(full_path, zip_file_path);
        } else {
            // Nếu là file, thêm file vào ZIP
            zip_source_t *source = zip_source_file(zip, full_path, 0, 0);
            if (source == NULL) {
                fprintf(stderr, "Error adding file %s to zip\n", full_path);
                zip_close(zip);
                closedir(dir);
                return -1;
            }

            // Thêm file vào archive
            if (zip_file_add(zip, entry->d_name, source, ZIP_FL_OVERWRITE) < 0) {
                fprintf(stderr, "Error adding file to zip archive: %s\n", zip_strerror(zip));
                zip_source_free(source);
                zip_close(zip);
                closedir(dir);
                return -1;
            }
        }
    }

    closedir(dir);
    zip_close(zip);
    return 0;
}

//===============================================================================================================================
void send_file(int sockfd, const char *data) {
    CHECK_DB_CONN(sockfd, "RECEIVE_FILE", conn);

    char *parsed_values[2];
    VALIDATE_INPUT(data, send_file_config, parsed_values, 2, "RECEIVE_FILE", sockfd, conn);
    char *file_path = parsed_values[0];
    char *token = parsed_values[1];

    int user_id = verify_jwt(token, config->SECRET_KEY, conn);
    if (user_id == -1) {
        SEND_ERROR(sockfd, "RECEIVE_FILE", 401, "Unauthorized", conn, NULL);
        goto cleanup;
    }

    char update_file_path[BUFFER_SIZE];
    snprintf(update_file_path, sizeof(update_file_path), "uploads/%d/%s", user_id, file_path);

    struct stat statbuf;
    if (stat(update_file_path, &statbuf) == 0 && S_ISDIR(statbuf.st_mode)) {
        char zip_file_path[BUFFER_SIZE];
        snprintf(zip_file_path, sizeof(zip_file_path), "%s.zip", update_file_path);
        
        if (zip_directory(update_file_path, zip_file_path) != 0) {
            SEND_ERROR(sockfd, "RECEIVE_FILE", 500, "Failed to zip folder", conn, NULL);
            goto cleanup;
        }
        snprintf(update_file_path, sizeof(update_file_path), "%s", zip_file_path);
    }

    FILE *file = fopen(update_file_path, "rb");
    if (file == NULL) {
        SEND_ERROR(sockfd, "RECEIVE_FILE", 404, "File không tồn tại", conn, NULL);
        goto cleanup;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    const size_t chunk_size = BUFFER_SIZE;
    char buffer[BUFFER_SIZE]; 

    int num_chunks = (int)((file_size + chunk_size - 1) / chunk_size);

    char response[BUFFER_SIZE];
    snprintf(response, sizeof(response), "SEND_FILE|REQUEST|0|%d", num_chunks);

    if (send(sockfd, response, strlen(response), 0) == -1) {
        perror("Lỗi khi gửi tín hiệu yêu cầu");
        goto cleanup_file;
    }

    char ack[4];
    ssize_t ack_received = recv(sockfd, ack, sizeof(ack) - 1, 0);
    if (ack_received < 0 || strncmp(ack, "ACK", 3) != 0) {
        fprintf(stderr, "Lỗi: Không nhận được tín hiệu ACK từ client.\n");
        SEND_ERROR(sockfd, "RECEIVE_FILE", 400, "Bad Request - ACK not received", conn, NULL);
        goto cleanup_file;
    }

    int chunk_index = 0;
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, chunk_size, file)) > 0) {
        if (send(sockfd, buffer, bytes_read, 0) == -1) {
            perror("Lỗi khi gửi chunk");
            SEND_ERROR(sockfd, "RECEIVE_FILE", 500, "Internal Server Error", conn, NULL);
            goto cleanup_file;
        }

        ack_received = recv(sockfd, ack, sizeof(ack) - 1, 0);
        if (ack_received < 0 || strncmp(ack, "ACK", 3) != 0) {
            fprintf(stderr, "Lỗi: Không nhận được tín hiệu ACK từ client.\n");
            SEND_ERROR(sockfd, "RECEIVE_FILE", 400, "Bad Request - ACK not received", conn, NULL);
            goto cleanup_file;
        }
        chunk_index++;
    }

    fclose(file);
    
    if (strstr(update_file_path, ".zip") != NULL) {
        if (remove(update_file_path) != 0) {
            perror("Lỗi khi xóa file zip");
        }
    }

    // ack_received = recv(sockfd, ack, sizeof(ack) - 1, 0);
    // if (ack_received < 0 || strncmp(ack, "ACK", 3) != 0) {
    //     fprintf(stderr, "Lỗi: Không nhận được tín hiệu ACK từ client.\n");
    //     SEND_ERROR(sockfd, "RECEIVE_FILE", 400, "Bad Request - ACK not received", conn, NULL);
    //     goto cleanup_file;
    // }

    char success_message[BUFFER_SIZE];
    snprintf(success_message, sizeof(success_message), "RECEIVE_FILE|SUCCESS|200|%s", file_path);
    if (send(sockfd, success_message, strlen(success_message), 0) < 0) {
        perror("Lỗi khi gửi thông điệp thành công");
        SEND_ERROR(sockfd, "RECEIVE_FILE", 500, "Internal Server Error", conn, NULL);
    }

cleanup:
    free_parsed_values(parsed_values, 2);
    return;

cleanup_file:
    fclose(file);
    goto cleanup;
}
