#ifndef AUTH_H
#define AUTH_H

#define BUFFER_SIZE 1024

void signup(int sockfd, const char *email, const char *username, const char *password);
void login(int sockfd, const char *email, const char *password);
void logout(int sockfd);
void change_password(int sockfd, char *old_password, char *new_password);

#endif



