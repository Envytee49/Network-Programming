#ifndef MANAGE_PATHS_H
#define MANAGE_PATHS_H

#define BUFFER_SIZE 1024

void rename_path(int sockfd, const char *path, const char *name);
void move_path(int sockfd, const char *path, const char *new_path);
void delete_path(int sockfd, const char *path);

#endif



