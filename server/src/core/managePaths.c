#include <stdio.h>
#include <string.h>
#include <unistd.h> 
#include <sys/socket.h>
#include <errno.h>
#include <dirent.h>
#include <sodium.h>
#include <sys/stat.h>
#include <stdbool.h>
#include "../../include/core/auth.h"
#include "../../include/database.h"
#include "../../include/config.h"
#include "../../include/utils/jwt_utils.h"
#include "../../include/utils/checkInput_utils.h"
#include "../../include/utils/macro_utils.h"

#define BUFFER_SIZE 1024 

//---------------------------------------------------------------------------------------------
void rename_path(int sockfd, const char *data) {
    CHECK_DB_CONN(sockfd, "RENAME", conn);
    char *parsed_values[3];
    VALIDATE_INPUT(data, rename_config, parsed_values, 3, "RENAME", sockfd, conn);

    char *path = parsed_values[0], *name = parsed_values[1], *token = parsed_values[2];

    int user_id = verify_jwt(token, config->SECRET_KEY, conn);
    if (user_id == -1) {
        SEND_ERROR(sockfd, "RENAME", 401, "Unauthorized", conn, NULL);
        free_parsed_values(parsed_values, 3);
        disconnect_db(conn);
        return;
    }

    // Lấy phần đường dẫn cha (folder/folder1/)
    char parent_path[BUFFER_SIZE];

    // Tìm dấu '/' cuối cùng
    char *last_slash = strrchr(path, '/');
    if (last_slash == NULL) {
        // Nếu không có dấu '/', set parent_path là chuỗi rỗng
        parent_path[0] = '\0';  // Chuỗi rỗng
    } else {
        // Nếu có dấu '/', lấy phần đường dẫn cha
        size_t parent_path_len = last_slash - path + 1;
        strncpy(parent_path, path, parent_path_len);
        parent_path[parent_path_len] = '\0';  // Đảm bảo chuỗi kết thúc bằng NULL
    }
    
    // Tạo đường dẫn mới
    char new_path[BUFFER_SIZE];
    snprintf(new_path, sizeof(new_path), "uploads/%d/%s%s", user_id, parent_path, name);

    // Đường dẫn cũ đầy đủ
    char full_path[BUFFER_SIZE];
    snprintf(full_path, sizeof(full_path), "uploads/%d/%s", user_id, path);

    // Kiểm tra sự tồn tại của tệp tin hoặc thư mục cũ
    if (access(full_path, F_OK) != 0) {
        // Nếu không tồn tại, gửi thông báo lỗi
        SEND_ERROR(sockfd, "RENAME", 404, "Not Found", conn, NULL);
        free_parsed_values(parsed_values, 3);
        disconnect_db(conn);
        return;
    }

    // Kiểm tra sự tồn tại của tệp tin hoặc thư mục mới (nếu cần)
    if (access(new_path, F_OK) == 0) {
        // Nếu tệp đã tồn tại, gửi thông báo lỗi
        SEND_ERROR(sockfd, "RENAME", 409, "Conflict", conn, NULL);
        free_parsed_values(parsed_values, 3);
        disconnect_db(conn);
        return;
    }

    //Đổi tên
    if (rename(full_path, new_path) != 0) {
        perror("Lỗi khi đổi tên");
        SEND_ERROR(sockfd, "RENAME", 500, "Internal Server Error", conn, NULL);
    } else {
        send(sockfd, "RENAME|SUCCESS|200", 20, 0);
    }

    printf("\nfull_path: %s", full_path);
    printf("\nnew_path: %s", new_path);

    disconnect_db(conn);
    free_parsed_values(parsed_values, 3);
}

//---------------------------------------------------------------------------------------------
void move_path(int sockfd, const char *data) {
    CHECK_DB_CONN(sockfd, "MOVE", conn);
    char *parsed_values[3];
    VALIDATE_INPUT(data, move_config, parsed_values, 3, "MOVE", sockfd, conn);

    char *path = parsed_values[0], *new_path = parsed_values[1], *token = parsed_values[2];

    int user_id = verify_jwt(token, config->SECRET_KEY, conn);
    if (user_id == -1) {
        SEND_ERROR(sockfd, "MOVE", 401, "Unauthorized", conn, NULL);
        free_parsed_values(parsed_values, 3);
        disconnect_db(conn);
        return;
    }

    // Đường dẫn cũ đầy đủ
    char full_path[BUFFER_SIZE];
    snprintf(full_path, sizeof(full_path), "uploads/%d/%s", user_id, path);

    // Tách tên tệp từ đường dẫn cũ và tạo đường dẫn mới cho tệp đích
    char *filename = strrchr(path, '/');  // Tìm dấu '/' cuối cùng
    if (filename == NULL) {
        filename = path;  // Nếu không tìm thấy '/', tên tệp là toàn bộ chuỗi
    } else {
        filename++;  // Bỏ qua dấu '/' để lấy tên tệp
    }

    // Tạo đường dẫn mới cho tệp đích
    char new_full_path[BUFFER_SIZE];
    if (snprintf(new_full_path, sizeof(new_full_path), "uploads/%d/%s/%s", user_id, new_path, filename) >= (int)sizeof(new_full_path)) {
        fprintf(stderr, "Đường dẫn mới quá dài\n");
        SEND_ERROR(sockfd, "MOVE", 400, "Bad Request", conn, NULL);
        free_parsed_values(parsed_values, 3);
        disconnect_db(conn);
        return;
    }

    // Kiểm tra sự tồn tại của tệp nguồn
    if (access(full_path, F_OK) != 0) {
        SEND_ERROR(sockfd, "MOVE", 404, "Not Found", conn, NULL);
        free_parsed_values(parsed_values, 3);
        disconnect_db(conn);
        return;
    }

    // Kiểm tra sự tồn tại của tệp đích
    if (access(new_full_path, F_OK) == 0) {
        SEND_ERROR(sockfd, "MOVE", 409, "Conflict", conn, NULL);
        free_parsed_values(parsed_values, 3);
        disconnect_db(conn);
        return;
    }

    // Di chuyển tệp
    if (rename(full_path, new_full_path) != 0) {
        perror("Lỗi khi di chuyển tệp");
        SEND_ERROR(sockfd, "MOVE", 500, "Internal Server Error", conn, NULL);
    } else {
        send(sockfd, "MOVE|SUCCESS|200", 18, 0);
    }

    disconnect_db(conn);
    free_parsed_values(parsed_values, 3);
}

//---------------------------------------------------------------------------------------------
void copy_path(int sockfd, const char *data) {
    CHECK_DB_CONN(sockfd, "COPY_FILE", conn);
    char *parsed_values[3];
    VALIDATE_INPUT(data, move_config, parsed_values, 3, "COPY_FILE", sockfd, conn);

    char *path = parsed_values[0], *new_path = parsed_values[1], *token = parsed_values[2];

    int user_id = verify_jwt(token, config->SECRET_KEY, conn);
    if (user_id == -1) {
        SEND_ERROR(sockfd, "COPY_FILE", 401, "Unauthorized", conn, NULL);
        free_parsed_values(parsed_values, 3);
        disconnect_db(conn);
        return;
    }

    // Full source path
    char full_path[BUFFER_SIZE];
    snprintf(full_path, sizeof(full_path), "uploads/%d/%s", user_id, path);

    // Extract filename from the source path
    char *filename = strrchr(path, '/');
    if (filename == NULL) {
        filename = path;
    } else {
        filename++;
    }

    // Create destination path
    char new_full_path[BUFFER_SIZE];
    if (snprintf(new_full_path, sizeof(new_full_path), "uploads/%d/%s/%s", user_id, new_path, filename) >= (int)sizeof(new_full_path)) {
        fprintf(stderr, "Destination path too long\n");
        SEND_ERROR(sockfd, "COPY_FILE", 400, "Bad Request", conn, NULL);
        free_parsed_values(parsed_values, 3);
        disconnect_db(conn);
        return;
    }

    // Check if source file exists
    if (access(full_path, F_OK) != 0) {
        SEND_ERROR(sockfd, "COPY_FILE", 404, "Not Found", conn, NULL);
        free_parsed_values(parsed_values, 3);
        disconnect_db(conn);
        return;
    }

    // Check if destination file already exists
    if (access(new_full_path, F_OK) == 0) {
        SEND_ERROR(sockfd, "COPY_FILE", 409, "Conflict", conn, NULL);
        free_parsed_values(parsed_values, 3);
        disconnect_db(conn);
        return;
    }

    // Copy the file
    FILE *src_file = fopen(full_path, "rb");
    if (src_file == NULL) {
        perror("Error opening source file");
        SEND_ERROR(sockfd, "COPY_FILE", 500, "Internal Server Error", conn, NULL);
        free_parsed_values(parsed_values, 3);
        disconnect_db(conn);
        return;
    }

    FILE *dest_file = fopen(new_full_path, "wb");
    if (dest_file == NULL) {
        perror("Error creating destination file");
        fclose(src_file);
        SEND_ERROR(sockfd, "COPY_FILE", 500, "Internal Server Error", conn, NULL);
        free_parsed_values(parsed_values, 3);
        disconnect_db(conn);
        return;
    }

    char buffer[BUFFER_SIZE];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), src_file)) > 0) {
        if (fwrite(buffer, 1, bytes_read, dest_file) != bytes_read) {
            perror("Error writing to destination file");
            fclose(src_file);
            fclose(dest_file);
            SEND_ERROR(sockfd, "COPY_FILE", 500, "Internal Server Error", conn, NULL);
            free_parsed_values(parsed_values, 3);
            disconnect_db(conn);
            return;
        }
    }

    fclose(src_file);
    fclose(dest_file);

    send(sockfd, "COPY_FILE|SUCCESS|200", 23, 0);

    disconnect_db(conn);
    free_parsed_values(parsed_values, 3);
}

void copy_dir_contents(const char *src, const char *dest, int sockfd) {
    DIR *dir = opendir(src);
    if (!dir) {
        perror("Error opening source directory");
        SEND_ERROR(sockfd, "COPY_FOLDER", 500, "Internal Server Error", NULL, NULL);
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char src_path[BUFFER_SIZE], dest_path[BUFFER_SIZE];
        snprintf(src_path, sizeof(src_path), "%s/%s", src, entry->d_name);
        snprintf(dest_path, sizeof(dest_path), "%s/%s", dest, entry->d_name);

        struct stat entry_stat;
        if (stat(src_path, &entry_stat) == 0) {
            if (S_ISDIR(entry_stat.st_mode)) {
                // Create subdirectory and recursively copy contents
                mkdir(dest_path, entry_stat.st_mode);
                copy_dir_contents(src_path, dest_path, sockfd);
            } else {
                // Copy file
                FILE *src_file = fopen(src_path, "rb");
                if (!src_file) {
                    perror("Error opening source file");
                    SEND_ERROR(sockfd, "COPY_FOLDER", 500, "Internal Server Error", NULL, NULL);
                    closedir(dir);
                    return;
                }

                FILE *dest_file = fopen(dest_path, "wb");
                if (!dest_file) {
                    perror("Error creating destination file");
                    fclose(src_file);
                    SEND_ERROR(sockfd, "COPY_FOLDER", 500, "Internal Server Error", NULL, NULL);
                    closedir(dir);
                    return;
                }

                char buffer[BUFFER_SIZE];
                size_t bytes_read;
                while ((bytes_read = fread(buffer, 1, sizeof(buffer), src_file)) > 0) {
                    if (fwrite(buffer, 1, bytes_read, dest_file) != bytes_read) {
                        perror("Error writing to destination file");
                        fclose(src_file);
                        fclose(dest_file);
                        SEND_ERROR(sockfd, "COPY_FOLDER", 500, "Internal Server Error", NULL, NULL);
                        closedir(dir);
                        return;
                    }
                }

                fclose(src_file);
                fclose(dest_file);
            }
        }
    }

    closedir(dir);
}

void copy_folder(int sockfd, const char *data) {
    CHECK_DB_CONN(sockfd, "COPY_FOLDER", conn);
    char *parsed_values[3];
    VALIDATE_INPUT(data, move_config, parsed_values, 3, "COPY_FOLDER", sockfd, conn);

    char *path = parsed_values[0], *new_path = parsed_values[1], *token = parsed_values[2];

    int user_id = verify_jwt(token, config->SECRET_KEY, conn);
    if (user_id == -1) {
        SEND_ERROR(sockfd, "COPY_FOLDER", 401, "Unauthorized", conn, NULL);
        free_parsed_values(parsed_values, 3);
        disconnect_db(conn);
        return;
    }

    // Full source path
    char full_path[BUFFER_SIZE];
    snprintf(full_path, sizeof(full_path), "uploads/%d/%s", user_id, path);
    char *filename = strrchr(path, '/');
    if (filename == NULL) {
        filename = path;
    } else {
        filename++;
    }
    // Full destination path
    char new_full_path[BUFFER_SIZE];
    // snprintf(new_full_path, sizeof(new_full_path), "uploads/%d/%s", user_id, new_path);
    snprintf(new_full_path, sizeof(new_full_path), "uploads/%d/%s/%s", user_id, new_path, filename) >= (int)sizeof(new_full_path);
    // Check if source directory exists
    struct stat src_stat;
    if (stat(full_path, &src_stat) != 0 || !S_ISDIR(src_stat.st_mode)) {
        SEND_ERROR(sockfd, "COPY_FOLDER", 404, "Source Not Found", conn, NULL);
        free_parsed_values(parsed_values, 3);
        disconnect_db(conn);
        return;
    }

    // Check if destination directory already exists
    struct stat dest_stat;
    if (stat(new_full_path, &dest_stat) == 0) {
        SEND_ERROR(sockfd, "COPY_FOLDER", 409, "Destination Already Exists", conn, NULL);
        free_parsed_values(parsed_values, 3);
        disconnect_db(conn);
        return;
    }

    // Create destination directory
    if (mkdir(new_full_path, src_stat.st_mode) != 0) {
        perror("Error creating destination directory");
        SEND_ERROR(sockfd, "COPY_FOLDER", 500, "Internal Server Error", conn, NULL);
        free_parsed_values(parsed_values, 3);
        disconnect_db(conn);
        return;
    }

    // Start copying contents
    copy_dir_contents(full_path, new_full_path, sockfd);

    send(sockfd, "COPY_FOLDER|SUCCESS|200", 23, 0);

    disconnect_db(conn);
    free_parsed_values(parsed_values, 3);
}
//---------------------------------------------------------------------------------------------
void delete_path(int sockfd, const char *data) {
    CHECK_DB_CONN(sockfd, "DELETE", conn);
    char *parsed_values[2];
    VALIDATE_INPUT(data, delete_config, parsed_values, 2, "DELETE", sockfd, conn);

    char *path = parsed_values[0], *token = parsed_values[1];

    int user_id = verify_jwt(token, config->SECRET_KEY, conn);
    if (user_id == -1) {
        SEND_ERROR(sockfd, "DELETE", 401, "Unauthorized", conn, NULL);
        free_parsed_values(parsed_values, 2);
        disconnect_db(conn);
        return;
    }

    // Cập nhật đường dẫn tệp
    char full_path[BUFFER_SIZE];
    if (snprintf(full_path, sizeof(full_path), "uploads/%d/%s", user_id, path) >= (int)sizeof(full_path)) {
        fprintf(stderr, "Đường dẫn file quá dài\n");
        SEND_ERROR(sockfd, "DELETE", 400, "Bad Request", conn, NULL);
        free_parsed_values(parsed_values, 2);
        disconnect_db(conn);
        return;
    }

    // Kiểm tra sự tồn tại của tệp hoặc thư mục
    if (access(full_path, F_OK) != 0) {
        // Nếu không tồn tại, gửi thông báo lỗi
        SEND_ERROR(sockfd, "DELETE", 404, "Not Found", conn, NULL);
        free_parsed_values(parsed_values, 2);
        disconnect_db(conn);
        return;
    }

   // Xóa tệp hoặc thư mục, sẽ không báo lỗi nếu tệp có thể xóa được
    if (remove(full_path) != 0) {
        // Nếu không xóa được tệp, thử xóa thư mục nếu đó là thư mục
        if (remove_dir(full_path) != 0) {
            // Nếu không xóa được thư mục hoặc tệp
            perror("Lỗi khi xóa tệp");
            SEND_ERROR(sockfd, "DELETE", 500, "Internal Server Error", conn, NULL);
        } else {
            send(sockfd, "DELETE|SUCCESS|200", 20, 0);
        }
    } else {
        send(sockfd, "DELETE|SUCCESS|200", 20, 0);
    }

    disconnect_db(conn);
    free_parsed_values(parsed_values, 2);
}

//---------------------------------------------------------------------------------------------
// Hàm xóa thư mục không rỗng sử dụng stat()
int remove_dir(const char *path) {
    DIR *d = opendir(path);
    if (d == NULL) {
        return -1;  // Không thể mở thư mục
    }

    struct dirent *entry;
    while ((entry = readdir(d)) != NULL) {
        // Bỏ qua các thư mục "." và ".."
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char full_path[BUFFER_SIZE];
        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);

        struct stat statbuf;
        if (stat(full_path, &statbuf) == -1) {
            closedir(d);
            return -1;  // Không thể lấy thông tin tệp
        }

        // Kiểm tra nếu là thư mục thì gọi đệ quy
        if (S_ISDIR(statbuf.st_mode)) {
            // Đệ quy để xóa thư mục con
            if (remove_dir(full_path) != 0) {
                closedir(d);
                return -1;  // Xóa thư mục con không thành công
            }
        } else {
            // Nếu là tệp, xóa nó
            if (remove(full_path) != 0) {
                closedir(d);
                return -1;  // Không thể xóa tệp
            }
        }
    }

    closedir(d);
    return rmdir(path);  // Xóa thư mục sau khi đã xóa tất cả các tệp trong đó
}