#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <errno.h>
#include <dirent.h>
#include "../../include/core/file.h"
#include "../../include/database.h"
#include "../../include/config.h" 
#include "../../include/utils/jwt_utils.h"
#include "../../include/utils/checkInput_utils.h"
#include "../../include/utils/macro_utils.h"
#include "../../include/utils/gzip_utils.h"

#define BUFFER_SIZE 1024  // Kích thước buffer 1KB

//===============================================================================================================================
void receive_folder(int sockfd, const char *data) {
    CHECK_DB_CONN(sockfd, "SEND_FOLDER", conn);

    //--------------------------------------------------------------------------------------
    // Tách giá trị từ dữ liệu đầu vào
    char *parsed_values[2];
    VALIDATE_INPUT(data, receive_folder_config, parsed_values, 2, "SEND_FOLDER", sockfd, conn);
    char *folder_name = parsed_values[0];
    char *token = parsed_values[1];

    //--------------------------------------------------------------------------------------
    // Xác thực người dùng qua token
    int user_id = verify_jwt(token, config->SECRET_KEY, conn);
    if (user_id == -1) {
        SEND_ERROR(sockfd, "SEND_FOLDER", 401, "Unauthorized", conn, NULL);
        free_parsed_values(parsed_values, 2);
        return;
    }

    //--------------------------------------------------------------------------------------
    // Kiểm tra và tạo folder 
    char folder_path[512];                                      // Tăng kích thước vùng đệm
    snprintf(folder_path, sizeof(folder_path), "uploads/%d/%s", user_id, folder_name);

    int folder_index = 0;
    while (access(folder_path, F_OK) == 0) {                    // Kiểm tra nếu folder đã tồn tại
        folder_index++;
        snprintf(folder_path, sizeof(folder_path), "uploads/%d/%s(%d)", user_id, folder_name, folder_index); // Cập nhật lại đường dẫn thư mục

        // Kiểm tra nếu vượt quá kích thước tối đa
        if (folder_index > 1000) {                              // Giới hạn số lần thử
            SEND_ERROR(sockfd, "SEND_FOLDER", 500, "Too many folder name conflicts", conn, NULL);
            free_parsed_values(parsed_values, 2);
            return;
        }
    }

    // Tạo thư mục mới
    if (mkdir(folder_path, 0777) != 0) { 
        perror("mkdir");
        SEND_ERROR(sockfd, "SEND_FOLDER", 500, "Failed to create folder", conn, NULL);
        free_parsed_values(parsed_values, 2);
        return;
    }

    // Gửi phản hồi thành công
    char success_message[BUFFER_SIZE];
    if (folder_index > 0) {       
        snprintf(success_message, sizeof(success_message), "SEND_FOLDER|SUCCESS|200|%s(%d)", folder_name, folder_index); 
    } else {
        snprintf(success_message, sizeof(success_message), "SEND_FOLDER|SUCCESS|200|%s", folder_name); 
    }

    if (send(sockfd, success_message, strlen(success_message), 0) < 0) {
        perror("Lỗi khi gửi thông điệp thành công");
        SEND_ERROR(sockfd, "SEND_FOLDER", 500, "Internal Server Error", conn, NULL);
    }

    // Giải phóng bộ nhớ sau khi hoàn thành
    free_parsed_values(parsed_values, 2);
}

//-------------------------------------------------------------------------------------------------------------------------------
void send_folder(int sockfd, const char *data) {
    // CHECK_DB_CONN(sockfd, "RECEIVE_FOLDER", conn);

    // //--------------------------------------------------------------------------------------
    // // Tách giá trị từ dữ liệu đầu vào
    // char *parsed_values[2];
    // VALIDATE_INPUT(data, send_folder_config, parsed_values, 2, "RECEIVE_FOLDER", sockfd, conn);
    // char *folder_path = parsed_values[0];
    // char *token = parsed_values[1];

    // //--------------------------------------------------------------------------------------
    // // Xác thực người dùng qua token
    // int user_id = verify_jwt(token, config->SECRET_KEY, conn);
    // if (user_id == -1) {
    //     SEND_ERROR(sockfd, "RECEIVE_FOLDER", 401, "Unauthorized", conn, NULL);
    //     free_parsed_values(parsed_values, 2);
    //     return;
    // }

    // //--------------------------------------------------------------------------------------
    // // Cập nhật đường dẫn folder
    // char update_folder_path[BUFFER_SIZE];
    // snprintf(update_folder_path, sizeof(update_folder_path), "uploads/%s", folder_path);

    // // Kiểm tra xem đường dẫn có phải là một file không (không phải thư mục)
    // struct stat statbuf;
    // if (stat(update_folder_path, &statbuf) == 0 && S_ISREG(statbuf.st_mode)) {
    //     SEND_ERROR(sockfd, "RECEIVE_FOLDER", 500, "The path is a file, not a directory", conn, NULL);
    //     return;
    // }

    // // Kiểm tra xem thư mục có tồn tại không
    // DIR *d = opendir(update_folder_path);  // Mở thư mục để kiểm tra
    // if (d == NULL) {
    //     SEND_ERROR(sockfd, "RECEIVE_FOLDER", 500, "Directory does not exist", conn, NULL);
    //     return;
    // }
    // closedir(d); // Đóng thư mục sau khi kiểm tra

    // // Tách tên thư mục từ đường dẫn
    // char *folder_name = strrchr(folder_path, '/');
    // folder_name = folder_name ? folder_name + 1 : folder_path;  // Lấy tên folder nếu có

    // // Mã hóa folder thành file zip
    // char zip_file_path[BUFFER_SIZE];
    // snprintf(zip_file_path, sizeof(zip_file_path), "%s.zip", update_folder_path);
    
    // // Giả sử bạn có một hàm tạo file zip, bạn sẽ gọi hàm này
    // int zip_status = zip_folder_to_file(update_folder_path, zip_file_path); 
    // if (zip_status != 0) {
    //     SEND_ERROR(sockfd, "RECEIVE_FOLDER", 500, "Failed to zip folder", conn, NULL);
    //     return;
    // }

    // // Gửi file zip qua hàm send_file
    // send_file(sockfd, zip_file_path);

    // // Gửi thông báo thành công
    // char success_message[BUFFER_SIZE];
    // snprintf(success_message, sizeof(success_message), "RECEIVE_FOLDER|SUCCESS|200|%s", folder_name);
    // success_message[sizeof(success_message) - 1] = '\0'; // Đảm bảo kết thúc đúng
    // if (send(sockfd, success_message, strlen(success_message), 0) < 0) {
    //     perror("Lỗi gửi phản hồi thành công");
    // }

    // free_parsed_values(parsed_values, 2);
}

//-------------------------------------------------------------------------------------------------------------------------------
// void getFolders(int sockfd, const char *data) { 
//     CHECK_DB_CONN(sockfd, "GET_FOLDERS", conn);

//     // Tách giá trị từ dữ liệu đầu vào
//     char *parsed_values[1];
//     VALIDATE_INPUT(data, get_folders_config, parsed_values, 1, "GET_FOLDERS", sockfd, conn);
//     char *token = parsed_values[0];

//     // Xác thực người dùng qua token
//     int user_id = verify_jwt(token, config->SECRET_KEY, conn);
//     if (user_id == -1) {
//         SEND_ERROR(sockfd, "GET_FOLDERS", 401, "Unauthorized", conn, NULL);
//         free_parsed_values(parsed_values, 3);
//         return;
//     }

//     // Xây dựng đường dẫn thư mục
//     char path[MAX_PATH_LENGTH];
//     snprintf(path, sizeof(path), "uploads/%d/", user_id);

//     // Mã hóa cấu trúc thư mục
//     char buffer[BUFFER_SIZE] = "";
//     encode_directory(path, buffer);

//     // Gửi cấu trúc thư mục mã hóa cho client
//     char message[BUFFER_SIZE];
//     snprintf(message, sizeof(message), "GET_FOLDERS|SUCCESS|200|%s", buffer);
//     send(sockfd, message, strlen(message), 0);

//     // Giải phóng bộ nhớ
//     free_parsed_values(parsed_values, 1);
// }

