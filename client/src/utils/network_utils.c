#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/socket.h>
#include "utils/network_utils.h"

#define BUFFER_SIZE 1024

//-------------------------------------------------------------------------------------------------------------------------------
char* send_receive(int sockfd, const char *format, ...) {
   
    char *response = malloc(BUFFER_SIZE);                   // Cấp phát bộ nhớ để lưu trữ phản hồi từ server
    if (response == NULL) {
        perror("Lỗi cấp phát bộ nhớ");
        return NULL; 
    }

    char message[BUFFER_SIZE];
    va_list args;
    va_start(args, format);                                 // Khởi tạo danh sách đối số biến
  
    vsnprintf(message, sizeof(message), format, args);      // Xây dựng thông điệp theo định dạng được chỉ định
    
    send(sockfd, message, strlen(message), 0);              // Gửi thông điệp đến server

    int n = recv(sockfd, response, BUFFER_SIZE - 1, 0);     // Nhận phản hồi từ server
    if (n > 0) {
        response[n] = '\0'; 
    } else {
        printf("\nError receiving server response.\n");
    }

    va_end(args);                                           // Kết thúc việc xử lý danh sách đối số biến

    printf("Server response: %s\n", response); 
    return response; 
}

//-------------------------------------------------------------------------------------------------------------------------------
int check_token() {
    if (strlen(token) == 0) {
        printf("Chưa đăng nhập. Vui lòng đăng nhập để thực hiện hành động này.\n");
        return 0; 
    }
    return 1;  
}

//-------------------------------------------------------------------------------------------------------------------------------
int process_search_response(int sockfd, const char *response, const char *request_type) {
    int num_results;
    char response_pattern[256];

    snprintf(response_pattern, sizeof(response_pattern), "%s|SUCCESS|200|%%d", request_type);
    sscanf(response, response_pattern, &num_results);
   
    ssize_t bytes_sent_ack = send(sockfd, "ACK", 3, 0);
    if (bytes_sent_ack < 0) {
        perror("Lỗi gửi tín hiệu xác nhận");
        return -1; 
    }

    return num_results; 
}

//-------------------------------------------------------------------------------------------------------------------------------
void process_search_results(int sockfd, int num_results) {
    char response[BUFFER_SIZE];
    ssize_t bytes_received;
    ssize_t bytes_sent_ack;

    for (int i = 0; i < num_results; i++) {
        bytes_received = read(sockfd, response, sizeof(response) - 1);
        if (bytes_received < 0) {
            perror("Lỗi đọc kết quả");
            return;
        }
        response[bytes_received] = '\0';
        printf("%s\n", response);

        bytes_sent_ack = send(sockfd, "ACK", 3, 0);
        if (bytes_sent_ack < 0) {
            perror("Lỗi gửi tín hiệu xác nhận");
            return;
        }
    }
}

