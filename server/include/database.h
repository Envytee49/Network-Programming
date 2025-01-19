#ifndef DATABASE_H
#define DATABASE_H

#include <stdbool.h>
#include <postgresql/libpq-fe.h>

PGconn* connect_db();
void disconnect_db(PGconn* conn);
bool verify_user(PGconn* conn, const char* username, const char* password);
PGconn* check_db_connect(int sockfd, const char* request_type);

#endif
