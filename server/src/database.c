#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <libconfig.h>
#include "../include/database.h"
#include "../include/config.h" 

//-------------------------------------------------------------------------------------------------------------------------------
PGconn* connect_db() {
    
    char conninfo[1024];  
    snprintf(conninfo, sizeof(conninfo), "host=%s port=%s dbname=%s user=%s password=%s",
        config->DB_HOST, config->DB_PORT, config->DB_NAME, config->DB_USER, config->DB_PASS );

    PGconn* conn = PQconnectdb(conninfo);

    if (PQstatus(conn) != CONNECTION_OK) {
        printf("Không thể kết nối tới cơ sở dữ liệu");
        PQfinish(conn);
        return NULL;
    }

    return conn;
}

//-------------------------------------------------------------------------------------------------------------------------------
void disconnect_db(PGconn *conn) {
    if (conn) {
        PQfinish(conn);  
    }
}

//-------------------------------------------------------------------------------------------------------------------------------
bool verify_user(PGconn* conn, const char* username, const char* password) {
    const char *query = "SELECT COUNT(*) FROM users WHERE username = $1 AND password = $2";
    const char *paramValues[2] = {username, password};
    
    PGresult* res = PQexecParams(conn, query, 2, NULL, paramValues, NULL, NULL, 0);
    
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        fprintf(stderr, "Verify user failed: %s", PQerrorMessage(conn));
        PQclear(res);
        return false;
    }

    int user_count = atoi(PQgetvalue(res, 0, 0));
    PQclear(res);

    return user_count > 0;
}
//-------------------------------------------------------------------------------------------------------------------------------
PGconn* check_db_connect(int sockfd, const char* request_type) {
    PGconn* conn = connect_db();
    if (!conn) {
        char response[128];
        snprintf(response, sizeof(response), "%s|ERROR|501|Không thể kết nối đến cơ sở dữ liệu", request_type);
        send(sockfd, response, strlen(response), 0);
    }
    return conn;
}