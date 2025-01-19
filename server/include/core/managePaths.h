#ifndef MANAGE_PATHS_H
#define MANAGE_PATHS_H

void rename_path(int sockfd, const char *data);
void move_path(int sockfd, const char *data);
void delete_path(int sockfd, const char *data);

#endif
