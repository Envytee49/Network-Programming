#ifndef FILE_H
#define FILE_H

#define BUFFER_SIZE 1024

void process_send_file(int sockfd, char *file_path); 
void send_file(int sockfd, char *file_path_client, char *file_path_server);
void receive_file(int sockfd, char *file_path, int type);                       // Nếu type = 1, file sẽ được lưu với đuôi .zip.


#endif



