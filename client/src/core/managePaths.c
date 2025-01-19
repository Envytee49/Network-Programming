#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../../include/core/managePaths.h"
#include "../../include/utils/network_utils.h"
#include "../../include/config.h"

void rename_path(int sockfd, const char *path, const char *name) {
    send_receive(sockfd, "RENAME|REQUEST|0|%s|%s|%s", path, name, token);
}

void move_path(int sockfd, const char *path, const char *new_path) {
    send_receive(sockfd, "MOVE|REQUEST|0|%s|%s|%s", path, new_path, token);
}

void copy_file(int sockfd, const char *path, const char *new_path) {
    send_receive(sockfd, "COPY_FILE|REQUEST|0|%s|%s|%s", path, new_path, token);
}

void copy_folder(int sockfd, const char *path, const char *new_path) {
    send_receive(sockfd, "COPY_FOLDER|REQUEST|0|%s|%s|%s", path, new_path, token);
}

void delete_path(int sockfd, const char *path) {
    send_receive(sockfd, "DELETE|REQUEST|0|%s|%s", path, token);
}

