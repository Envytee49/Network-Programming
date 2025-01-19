#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "../include/user_actions.h"
#include "../include/config.h"

int main() {
    int sockfd = create_socket();
    if (sockfd < 0) {
        perror("Socket creation failed");
        return -1;
    }

    if (connect_to_server(sockfd, SERVER_IP, SERVER_PORT) < 0) {
        perror("Connection failed");
        close(sockfd);
        return -1;
    }

    printf("Connected to server at %s:%d\n", SERVER_IP, SERVER_PORT);
    handle_user_actions(sockfd);

    close(sockfd);
    return 0;
}
