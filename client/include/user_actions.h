#ifndef USER_ACTIONS_H
#define USER_ACTIONS_H

extern char token[255];

int create_socket();
int connect_to_server(int sockfd, const char *ip, int port);
void handle_user_actions(int sockfd);

#endif
