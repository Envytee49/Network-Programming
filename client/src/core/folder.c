#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/socket.h> 
#include <dirent.h>
#include "../../include/core/folder.h"
#include "../../include/core/file.h"
#include "../../include/utils/network_utils.h"

//===============================================================================================================================
void send_folder(int sockfd, char *folder_path_client, char *folder_path_server) {
    if (!check_token()) return;

    char request[BUFFER_SIZE];
    snprintf(request, sizeof(request), "SEND_FOLDER|REQUEST|0|%s|%s", folder_path_server, token);
    if (send(sockfd, request, strlen(request), 0) == -1) {
        perror("Lỗi khi gửi yêu cầu tạo folder");
        return;
    }

    char response[BUFFER_SIZE];
    int len = recv(sockfd, response, sizeof(response) - 1, 0);
    if (len == -1) {
        perror("Lỗi khi nhận phản hồi từ server");
        return;
    }
    response[len] = '\0';

    printf("Server response: %s\n", response);

    int result = sscanf(response, "SEND_FOLDER|SUCCESS|200|%s", folder_path_server);
    if (result != 1) {
        perror("Lỗi khi tách đường dẫn từ phản hồi");
        return;
    }

    DIR *d = opendir(folder_path_client);
    if (d == NULL) {
        perror("Lỗi khi mở thư mục");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(d)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char file_path_client[BUFFER_SIZE], file_path_server_new[BUFFER_SIZE];
        snprintf(file_path_client, sizeof(file_path_client), "%s/%s", folder_path_client, entry->d_name);
        snprintf(file_path_server_new, sizeof(file_path_server_new), "%s/%s", folder_path_server, entry->d_name);

        struct stat statbuf;
        if (stat(file_path_client, &statbuf) == 0) {
            if (S_ISDIR(statbuf.st_mode)) {
                send_folder(sockfd, file_path_client, file_path_server_new);
            } else if (S_ISREG(statbuf.st_mode)) {
                send_file(sockfd, file_path_client, file_path_server_new);
            }
        } else {
            perror("Lỗi khi truy cập file/thư mục");
        }
    }
    closedir(d);
}

//===============================================================================================================================
void receive_folder(int sockfd, char *folder_path) {
    receive_file(sockfd, folder_path, 1);   // Gọi hàm receive_file với type = 1 để nhận folder dưới dạng file nén (.zip).
}


//===============================================================================================================================
// Hàm xử lý phản hồi GET_FOLDERS từ server
//void getFolders(int sockfd) {
    // Gửi yêu cầu và nhận phản hồi từ server
    //char *response = send_receive(sockfd, "GET_FOLDERS|REQUEST|0|%s", token);
    // if (response != NULL) {
    //     // Phân tích phản hồi nhận được theo định dạng "GET_FOLDERS|SUCCESS|200|%s"
    //     char *status = strtok(response, "|");
    //     char *message = strtok(NULL, "|");
    //     char *code = strtok(NULL, "|");
    //     char *compressed_path = strtok(NULL, "|");

    //     // Kiểm tra xem phản hồi có hợp lệ không
    //     if (status && strcmp(status, "GET_FOLDERS") == 0 &&
    //         message && strcmp(message, "SUCCESS") == 0 &&
    //         code && strcmp(code, "200") == 0 &&
    //         compressed_path) {
            
    //         // Giải nén dữ liệu
    //         char decompressed_path[BUFFER_SIZE];
    //         int decompressed_size = decompress_buffer(compressed_path, strlen(compressed_path), decompressed_path);

    //         if (decompressed_size > 0) {
    //             // Hiển thị cấu trúc thư mục giải nén
    //             printf("Directory Structure:\n");
    //             print_directory_tree(decompressed_path);
    //         }
    //     } else {
    //         printf("Error: Invalid response from server\n");
    //     }
    //free(response); 
    // }
//}

//===============================================================================================================================
void process_send_folder(int sockfd, char *folder_path) {
    if (!check_token()) return;                     // Kiểm tra token trước khi tiếp tục

    // Cập nhật đường dẫn đầy đủ đến folder
    char folder_path_client[BUFFER_SIZE];
    snprintf(folder_path_client, sizeof(folder_path_client), "myDirectory/%s", folder_path);

    // Kiểm tra nếu đường dẫn là file (không phải thư mục)
    struct stat statbuf;
    if (stat(folder_path_client, &statbuf) == 0 && S_ISREG(statbuf.st_mode)) {
        // Nếu là file, in lỗi và thoát
        printf("Đường dẫn '%s' là file, không phải folder.\n", folder_path);
        return;
    }

    // Kiểm tra folder có tồn tại không
    DIR *d = opendir(folder_path_client);  // Mở thư mục để kiểm tra
    if (d == NULL) {
        perror("Không thể mở folder");
        return;
    }

    // Lấy tên folder từ đường dẫn
    char *folder_name = strrchr(folder_path, '/');  // Tìm ký tự '/' cuối cùng
    if (folder_name != NULL) {
        folder_name++;  // Bỏ qua '/' để lấy tên folder
    } else {
        folder_name = folder_path;  // Nếu không có '/', dùng nguyên tên đường dẫn
    }

    send_folder(sockfd, folder_path_client, folder_name);       // Gọi hàm send_folder để thực hiện gửi folder
}
