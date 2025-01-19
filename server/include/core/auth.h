#ifndef AUTH_H
#define AUTH_H

void signup(int sockfd, const char *data);
void login(int sockfd, const char *data);
void logout(int sockfd, const char *data);
void change_password(int sockfd, const char *data);

#endif
