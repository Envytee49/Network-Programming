#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <unistd.h>
#include "../include/handler.h"
#include "../include/config.h" 
#include "../include/database.h"

void *client_handler(void *client_fd_ptr) {
    int client_fd = *((int *)client_fd_ptr);
    handle_client(client_fd);
    free(client_fd_ptr);  
    return NULL;
}

int main() {
    init_config();  
    if (load_config("config.cfg", config) != 0) {
        perror("Configuration loading failed");
        exit(EXIT_FAILURE);
    }

    int server_fd, client_fd;
    struct sockaddr_in address;
    socklen_t addr_len = sizeof(address);
    pthread_t thread_id;

    // Tạo socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    // Thiết lập thông tin địa chỉ
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(config->PORT);  

    // Gắn kết socket với địa chỉ
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) == -1) {
        perror("Bind failed");
        close(server_fd);  
        exit(EXIT_FAILURE);
    }

    // Lắng nghe kết nối đến server
    if (listen(server_fd, 3) == -1) {
        perror("Listen failed");
        close(server_fd);  
        exit(EXIT_FAILURE);
    }

    printf("Server is running on port %d...\n", config->PORT);

    // Lắng nghe và xử lý kết nối client
    while ((client_fd = accept(server_fd, (struct sockaddr *)&address, &addr_len)) >= 0) {
        // Tạo một thread mới để xử lý client
        int *client_fd_ptr = malloc(sizeof(int));  // Cấp phát bộ nhớ cho client_fd
        if (client_fd_ptr == NULL) {
            perror("Memory allocation failed");
            close(client_fd);  // Nếu không cấp phát được bộ nhớ, đóng kết nối client
            continue;
        }

        *client_fd_ptr = client_fd;

        // Tạo luồng xử lý client
        if (pthread_create(&thread_id, NULL, client_handler, (void *)client_fd_ptr) != 0) {
            perror("Failed to create thread");
            free(client_fd_ptr);  // Giải phóng bộ nhớ khi không thể tạo thread
            close(client_fd);  // Đóng kết nối client
        } else {
            pthread_detach(thread_id);  // Tách thread để không cần join lại
        }
    }

    if (client_fd == -1) {
        perror("Accept failed");
    }

    // Nếu kết thúc vòng lặp accept, đóng socket server
    close(server_fd);
    return 0;
}