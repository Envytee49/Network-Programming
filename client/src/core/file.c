#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include "../../include/core/file.h"
#include "../../include/utils/network_utils.h"

//===============================================================================================================================
void send_file(int sockfd, char *file_path_client, char *file_path_server) {
    //Kiểm tra nếu tệp có tồn tại
    FILE *file = fopen(file_path_client, "rb");
    if (file == NULL) { perror("Error opening file"); return; }

    // Lấy kích thước của tệp
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Tính số chunk cần gửi
    int num_chunks = (int)((file_size + BUFFER_SIZE - 1) / BUFFER_SIZE);
    char buffer[BUFFER_SIZE]; 
    size_t bytes_read;

    // Chuẩn bị và gửi yêu cầu đến server
    char response[BUFFER_SIZE];
    snprintf(response, sizeof(response), "SEND_FILE|REQUEST|0|%s|%d|%s", file_path_server, num_chunks, token);

    if (send(sockfd, response, strlen(response), 0) == -1) {
        perror("Error sending request");
        fclose(file);
        return;
    }

    // Chờ nhận tín hiệu ACK từ server
    char ack[4];
    ssize_t ack_received = recv(sockfd, ack, sizeof(ack) - 1, 0);
    if (ack_received < 0 || strncmp(ack, "ACK", 3) != 0) {
        fprintf(stderr, "Error: No ACK received from server.\n");
        fclose(file);
        return;
    }

    // Gửi các chunk của tệp đến server
    int chunk_index = 0;
    while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {
        if (send(sockfd, buffer, bytes_read, 0) == -1) {
            perror("Error sending chunk");
            break;
        }

        // Chờ nhận ACK sau mỗi chunk
        ack_received = recv(sockfd, ack, sizeof(ack) - 1, 0);
        if (ack_received < 0 || strncmp(ack, "ACK", 3) != 0) {
            fprintf(stderr, "Error: No ACK for chunk from server.\n");
            break;
        }
        chunk_index++;
    }

    fclose(file);

    //Nhận phản hồi cuối cùng từ server
    char final_response[BUFFER_SIZE];
    if (recv(sockfd, final_response, sizeof(final_response) - 1, 0) > 0) {
        printf("Server response: %s\n", final_response);
    }
}

//===============================================================================================================================
void receive_file(int sockfd, char *file_path, int type) {
    if (!check_token()) return;

    // Tách tên file từ file_path
    char *file_name = strrchr(file_path, '/');  // Tìm dấu '/' cuối cùng trong file_path
    if (file_name == NULL) {
        // Nếu không tìm thấy '/', nghĩa là file_path là tên file không có đường dẫn
        file_name = file_path;
    } else {
        file_name++;  // Chuyển qua ký tự tiếp theo sau dấu '/'
    }

    // Gửi yêu cầu nhận file tới server
    char request[BUFFER_SIZE];
    snprintf(request, sizeof(request), "RECEIVE_FILE|REQUEST|0|%s|%s", file_path, token);

    if (send(sockfd, request, strlen(request), 0) == -1) {
        perror("Lỗi khi gửi yêu cầu nhận file");
        return;
    }

    // Nhận tín hiệu yêu cầu và số lượng chunk từ server
    char response[BUFFER_SIZE];
    ssize_t response_received = recv(sockfd, response, sizeof(response) - 1, 0);
    if (response_received < 0) {
        perror("Lỗi khi nhận phản hồi từ server");
        return;
    }
    response[response_received] = '\0';  // Đảm bảo chuỗi kết thúc đúng cách

    // Kiểm tra xem phản hồi có phải là thông điệp yêu cầu hợp lệ không
    int N = 0;
    if (sscanf(response, "SEND_FILE|REQUEST|0|%d", &N) != 1) {
        // Kiểm tra nếu phản hồi là lỗi
        if (strncmp(response, "RECEIVE_FILE|ERROR", 18) == 0) {
            printf("Lỗi nhận file: %s\n", response);
            return;
        }
        // Nếu không phải dạng yêu cầu hợp lệ, trả về thông báo lỗi
        printf("Phản hồi không hợp lệ từ server: %s\n", response);
        return;
    }

    // Gửi tín hiệu ACK để xác nhận đã nhận số lượng chunk
    if (send(sockfd, "ACK", 3, 0) < 0) {
        perror("Lỗi gửi tín hiệu ACK");
        return;
    }

    // Thêm đuôi zip vào file nếu hàm được gọi với tham số type=1
    if (type == 1) {
        char temp_name[BUFFER_SIZE];
        snprintf(temp_name, sizeof(temp_name), "%s.zip", file_name);
        strncpy(file_name, temp_name, sizeof(temp_name));               // Cập nhật lại file_name
    }

    // Đường dẫn thư mục lưu file
    char dir_path[22] = "myDirectory/downloads";

    // Xây dựng đường dẫn file
    char output_file[BUFFER_SIZE];
    snprintf(output_file, sizeof(output_file), "%s/%s", dir_path, file_name);

    // Kiểm tra nếu file đã tồn tại và tạo tên file mới với index
    struct stat statbuf;
    if (stat(output_file, &statbuf) == 0) {
        // Nếu file đã tồn tại, tách tên file và phần mở rộng
        char file_base[BUFFER_SIZE], file_ext[128];
        char *dot_pos = strrchr(file_name, '.');  // Tìm vị trí dấu chấm (.) để tách phần mở rộng

        if (dot_pos != NULL) {
            // Tách tên file và phần mở rộng
            size_t name_len = dot_pos - file_name;
            strncpy(file_base, file_name, name_len);
            file_base[name_len] = '\0';
            strncpy(file_ext, dot_pos, sizeof(file_ext));
        } else {
            // Nếu không có phần mở rộng, đặt phần mở rộng là rỗng
            strncpy(file_base, file_name, sizeof(file_base));
            file_ext[0] = '\0';
        }

        // Tạo tên file mới với index
        int index = 1;
        char new_file_path[BUFFER_SIZE];
        while (1) {
            // Tạo đường dẫn file mới với tên index
            snprintf(new_file_path, sizeof(new_file_path), "%s/%s(%d)%s", dir_path, file_base, index, file_ext);

            // Kiểm tra sự tồn tại của file mới
            if (stat(new_file_path, &statbuf) != 0) {
                // Nếu file chưa tồn tại, sử dụng tên mới
                strcpy(output_file, new_file_path);
                break;
            }
            index++;  // Nếu file đã tồn tại, thử lại với index tiếp theo
        }
    }

    // Mở file với tên mới
    FILE *file = fopen(output_file, "wb");
    if (file == NULL) {
        perror("Không thể mở file để ghi");
        return;
    }

    char buffer[BUFFER_SIZE];  
    ssize_t bytes_received;
    int chunks_received = 0;

    // Nhận từng chunk từ server
    while (chunks_received < N) {
        bytes_received = recv(sockfd, buffer, sizeof(buffer), 0);
        if (bytes_received < 0) {
            perror("Lỗi nhận dữ liệu từ server");
            break;
        }

        if (bytes_received == 0) {
            break;  // Server đã gửi xong file
        }

        fwrite(buffer, 1, bytes_received, file);  // Lưu dữ liệu vào file
        chunks_received++;

        // Gửi tín hiệu ACK để xác nhận
        if (send(sockfd, "ACK", 3, 0) < 0) {
            perror("Lỗi gửi tín hiệu ACK");
            break;
        }
    }

    // Đảm bảo file được đóng và tài nguyên được giải phóng
    fclose(file);

    // Nhận phản hồi cuối cùng từ server
    char server_response[BUFFER_SIZE];
    ssize_t final_response = recv(sockfd, server_response, sizeof(server_response) - 1, 0);
    if (final_response < 0) {
        perror("Lỗi nhận phản hồi cuối từ server");
    } else {
        server_response[final_response] = '\0';
        printf("Server response: %s\n", server_response);
    }
}

//===============================================================================================================================
void process_send_file(int sockfd, char *file_path) {
    // Kiểm tra token trước khi thực hiện (nếu token không hợp lệ, dừng lại)
    if (!check_token()) return;

    // Tạo đường dẫn đầy đủ của file trong thư mục myDirectory
    char file_path_client[BUFFER_SIZE];
    snprintf(file_path_client, sizeof(file_path_client), "myDirectory/%s", file_path);

    // Kiểm tra nếu đường dẫn trỏ tới một thư mục
    struct stat statbuf;
    if (stat(file_path_client, &statbuf) == 0 && S_ISDIR(statbuf.st_mode)) {
        // Nếu là thư mục, in thông báo lỗi và dừng lại
        printf("Đường dẫn '%s' là thư mục, không phải tệp.\n", file_path);
        return;
    }
  
    // Kiểm tra xem tệp có tồn tại hay không
    FILE *file = fopen(file_path_client, "rb");
    if (file == NULL) {
        perror("Không thể mở tệp");
        return;
    }
    fclose(file);

    // Trích xuất tên tệp từ đường dẫn
    char *file_name = strrchr(file_path, '/');  // Tìm ký tự '/' cuối cùng
    if (file_name != NULL) {
        file_name++;  // Bỏ qua '/' để lấy phần tên tệp
    } else {
        file_name = file_path;  // Nếu không có '/', dùng toàn bộ đường dẫn làm tên tệp
    }
    // Gọi send_file để thực hiện việc gửi tệp
    send_file(sockfd, file_path_client, file_name);
}
