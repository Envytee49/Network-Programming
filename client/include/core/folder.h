#ifndef FOLDER_H
#define FOLDER_H

#define BUFFER_SIZE 1024

void send_folder(int sockfd, char *folder_path_client, char *folder_path_server);
void process_send_folder(int sockfd, char *folder_path);    
void receive_folder(int sockfd, char *folder_path);

// void getFolders(int sockfd);

#endif


