#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <jwt.h>
#include <time.h>
#include <sodium.h>
#include "../../include/database.h"

//-------------------------------------------------------------------------------------------------------------------------------
int generate_jwt(const char* id, const char* secret_key, char** token, PGconn* conn) {
    jwt_t* jwt = NULL;
    time_t now = time(NULL);

    if (jwt_new(&jwt) != 0) return -1;
    jwt_add_grant(jwt, "id", id);
    jwt_add_grant_int(jwt, "iat", now);
    jwt_add_grant_int(jwt, "exp", now + 3600);  // Token hết hạn sau 1 giờ

    if (jwt_set_alg(jwt, JWT_ALG_HS256, (unsigned char*)secret_key, strlen(secret_key)) != 0) {
        jwt_free(jwt);
        return -1;
    }

    *token = jwt_encode_str(jwt);
    jwt_free(jwt);
    if (*token == NULL) return -1;

    const char* param_values[2] = { id, *token };
    PGresult* res = PQexecParams(conn, "INSERT INTO sessions (user_id, token) VALUES ($1, $2)", 2, NULL, param_values, NULL, NULL, 0);

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        PQclear(res);
        return -1;
    }

    PQclear(res);
    return 0;
}

//-------------------------------------------------------------------------------------------------------------------------------
int verify_jwt(const char *token, const char *secret_key, PGconn* conn) {
    const char* param_values[1] = { token };
    PGresult* res = PQexecParams(conn, "SELECT user_id FROM sessions WHERE token = $1", 1, NULL, param_values, NULL, NULL, 0);

    if (PQresultStatus(res) != PGRES_TUPLES_OK || PQntuples(res) == 0) {
        PQclear(res);
        return 0;
    }

    int user_id = atoi(PQgetvalue(res, 0, 0));      // Lấy `user_id` từ kết quả truy vấn
    PQclear(res);

    jwt_t* jwt = NULL;
    if (jwt_decode(&jwt, token, (unsigned char*)secret_key, strlen(secret_key)) != 0) {
        return 0;
    }

    // Kiểm tra thời gian hết hạn
    int exp_time = jwt_get_grant_int(jwt, "exp");
    if (exp_time < time(NULL)) {
        jwt_free(jwt);
        return 0;
    }

    jwt_free(jwt);

    return user_id;
}

//-------------------------------------------------------------------------------------------------------------------------------
int delete_jwt(const char* token, PGconn* conn) {
    const char* param_values[1] = { token };
    PGresult* res = PQexecParams(conn, "DELETE FROM sessions WHERE token = $1", 1, NULL, param_values, NULL, NULL, 0);

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        PQclear(res);
        return -1;
    }

    PQclear(res);
    return 0;
}

//-------------------------------------------------------------------------------------------------------------------------------
int hash_password(const char *password, char *hashed_password) {
    if (sodium_init() == -1) {
        return -1;                                      // Lỗi khởi tạo libsodium
    }

    if (crypto_pwhash_str(
            hashed_password,                            // Lưu kết quả hash ở đây
            password,                                   // Mật khẩu cần mã hóa
            strlen(password),                           // Độ dài mật khẩu
            crypto_pwhash_OPSLIMIT_SENSITIVE,           // Giới hạn về tài nguyên
            crypto_pwhash_MEMLIMIT_SENSITIVE) != 0) {   // Giới hạn bộ nhớ
        return -2;                                      // Lỗi khi mã hóa mật khẩu
    }

    return 0;                                           // Thành công
}

//-------------------------------------------------------------------------------------------------------------------------------
int verify_password(const char *password, const char *stored_hash) {
    if (crypto_pwhash_str_verify(stored_hash, password, strlen(password)) != 0) {
        return 0; 
    }
    return 1;
}
