#ifndef UTILS_H
#define UTILS_H

extern char token[255];

char* send_receive(int sockfd, const char *format, ...);
int check_token();

int process_search_response(int sockfd, const char *response, const char *request_type);
void process_search_results(int sockfd, int num_results);

#endif

