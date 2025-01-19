#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>  
#include "../include/user_actions.h"
#include "../include/config.h"
#include "../include/core/core.h"
#include "../include/utils/input_utils.h"

int create_socket() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
    }
    return sockfd;
}

int connect_to_server(int sockfd, const char *ip, int port) {
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip);
    return connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
}

void handle_user_actions(int sockfd) {
    int choice;

    while (1) {
        // printf("\033[2J\033[H"); 
        if (token[0] == '\0') {
            printf("\n==================Select action==================\n");
            printf("1. Sign up\n");
            printf("2. Login\n");
            printf("3. Exit\n");
            printf("Enter your choice: ");

            if (scanf("%d", &choice) != 1) {
                printf("Invalid input. Please enter a number.\n");
                while (getchar() != '\n');
                continue;
            }
            while (getchar() != '\n' && getchar() != EOF);

            switch (choice) {
                case 1: {
                    char email[255], username[64], password[128];
                    input_text_no_space("\tEnter email: ", email, sizeof(email));
                    input_text("\tEnter username: ", username, sizeof(username));
                    input_text("\tEnter password: ", password, sizeof(password));
                    signup(sockfd, email, username, password);
                    break;
                }
                case 2: {
                    char email[255], password[128];
                    input_text_no_space("\tEnter email: ", email, sizeof(email));
                    input_text("\tEnter password: ", password, sizeof(password));
                    login(sockfd, email, password);
                    break;
                }
                case 3: {
                    printf("Exiting client...\n");
                    return;
                }
                default: {
                    printf("Invalid choice. Please try again.\n");
                    break;
                }
            }
        } else {
            printf("\n==================Select action==================\n");
            printf("1. Logout\n");
            printf("2. Change password\n");
            printf("3. Upload file\n");
            printf("4. Download file\n");
            printf("5. Upload folder\n");
            printf("6. Download folder\n");
            printf("7. Copy file\n");
            printf("8. Copy folder\n");
            printf("9. Rename\n");
            printf("10. Delete\n");
            printf("11. Move\n");
            printf("12. Exit\n");
            printf("Enter your choice: ");

            if (scanf("%d", &choice) != 1) {
                printf("Invalid input. Please enter a number.\n");
                while (getchar() != '\n');
                continue;
            }
            while (getchar() != '\n' && getchar() != EOF);

            switch (choice) {
                case 1: {
                    logout(sockfd);
                    break;
                }
                case 2: {
                    char old_password[128], new_password[128];
                    input_text("\tEnter old password: ", old_password, sizeof(old_password));
                    input_text("\tEnter new password: ", new_password, sizeof(new_password));
                    change_password(sockfd, old_password, new_password);
                    break;
                }
                case 3: {
                    char file_path[255];
                    input_text("\tEnter file path: ", file_path, sizeof(file_path));    
                    process_send_file(sockfd, file_path);
                    break;
                }
                case 4: {
                    char file_path[255];
                    input_text("\tEnter file path: ", file_path, sizeof(file_path));
                    receive_file(sockfd, &file_path, 0);
                    break;
                }
                case 5: {
                    char folder_path[255];
                    input_text("\tEnter folder path: ", folder_path, sizeof(folder_path));
                    process_send_folder(sockfd, folder_path);
                    break;
                }
                case 6: {
                    char folder_path[255];
                    input_text("\tEnter folder path: ", folder_path, sizeof(folder_path));
                    receive_folder(sockfd, folder_path);
                    break;
                }
                case 7: {
                    char path[255], new_path[255];
                    input_text("\tEnter file path: ", path, sizeof(path));
                    input_text("\tEnter copy path: ", new_path, sizeof(new_path));
                    copy_file(sockfd, path, new_path);
                    break;
                }
                case 8: {
                    char path[255], new_path[255];
                    input_text("\tEnter folder path: ", path, sizeof(path));
                    input_text("\tEnter copy path: ", new_path, sizeof(new_path));
                    copy_folder(sockfd, path, new_path);
                    break;
                }
                 case 9: {
                    char path[255], name[255];
                    input_text("\tEnter folder/file path: ", path, sizeof(path));
                    input_text("\tEnter name: ", name, sizeof(name));
                    rename_path(sockfd, path, name);
                    break;
                }
                 case 10: {
                    char path[255];
                    input_text("\tEnter folder/file path: ", path, sizeof(path));
                    delete_path(sockfd, path);
                    break;
                }
                 case 11: {
                    char path[255], new_path[255];
                    input_text("\tEnter folder/file path: ", path, sizeof(path));
                    input_text("\tEnter new path: ", new_path, sizeof(new_path));
                    move_path(sockfd, path, new_path);
                    break;
                }
                case 12: {
                    printf("Exiting client...\n");
                    return;
                }
                default: {
                    printf("Invalid choice. Please try again.\n");
                    break;
                }
            }
        }
    }
}