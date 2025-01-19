#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include "../include/handler.h"
#include "../include/core/core.h"

#define BUFFER_SIZE 1024

void handle_client(int sockfd) {
    char buffer[BUFFER_SIZE];
    int n;

    while ((n = recv(sockfd, buffer, BUFFER_SIZE, 0)) > 0) {
        buffer[n] = '\0';  
        printf("Received: %s\n", buffer);

        char command[32], status[32], code[32], data[256];
        sscanf(buffer, "%31[^|]|%31[^|]|%31[^|]|%255[^\n]", command, status, code, data);

        if (strncmp(command, "SIGNUP", 6) == 0) {
            signup(sockfd, data);
        } else if (strncmp(command, "LOGIN", 5) == 0) {
            login(sockfd, data);
        } else if (strncmp(command, "LOGOUT", 6) == 0) {
            logout(sockfd, data);
        } else if (strncmp(command, "CHANGE_PASSWORD", 15) == 0) {
            change_password(sockfd, data);
        } else if (strncmp(command, "SEND_FILE", 9) == 0) {
            receive_file(sockfd, data);  
        } else if (strncmp(command, "RECEIVE_FILE", 12) == 0) {
            send_file(sockfd, data);  
        } else if (strncmp(command, "SEND_FOLDER", 11) == 0) {
            receive_folder(sockfd, data);  
        } else if (strncmp(command, "RECEIVE_FOLDER", 14) == 0) {
            send_folder(sockfd, data);  
        } else if (strncmp(command, "COPY_FILE", 9) == 0) {
            copy_path(sockfd, data);   
        } 
        else if (strncmp(command, "COPY_FOLDER", 11) == 0) {
            copy_folder(sockfd, data);   
        } 
        else if (strncmp(command, "RENAME", 6) == 0) {
            rename_path(sockfd, data);  
        } else if (strncmp(command, "MOVE", 4) == 0) {
            move_path(sockfd, data);  
        } else if (strncmp(command, "DELETE", 6) == 0) {
            delete_path(sockfd, data);  
        } else {
            send(sockfd, "ERROR|400|Command invalid", 32, 0);
        }
    }

    if (n == 0) {
        printf("Client disconnected\n");
    } else if (n < 0) {
        perror("recv failed");
    }

    close(sockfd);
}
