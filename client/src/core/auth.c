#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../../include/core/auth.h"
#include "../../include/utils/network_utils.h"
#include "../../include/config.h"

char token[255];

//===============================================================================================================================
void signup(int sockfd, const char *email, const char *username, const char *password) {
    send_receive(sockfd, "SIGNUP|REQUEST|0|%s|%s|%s", email, username, password);
}

//===============================================================================================================================
void login(int sockfd, const char *email, const char *password) {
    char* response = send_receive(sockfd, "LOGIN|REQUEST|0|%s|%s", email, password);
    if (response != NULL) {
        sscanf(response, "LOGIN|SUCCESS|%*d|%255s", token);  
        free(response); 
    }
}

//===============================================================================================================================
void logout(int sockfd) {
    if (!check_token()) return;
    send_receive(sockfd, "LOGOUT|REQUEST|0|%s", token);
    memset(token, 0, sizeof(token));  
}

//===============================================================================================================================
void change_password(int sockfd, char *old_password, char *new_password) {
    if (check_token()) send_receive(sockfd, "CHANGE_PASSWORD|REQUEST|0|%s|%s|%s", old_password, new_password, token);
}