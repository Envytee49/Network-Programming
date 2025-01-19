#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sodium.h>
#include <sys/stat.h>
#include <stdbool.h>
#include "../../include/core/auth.h"
#include "../../include/database.h"
#include "../../include/config.h"
#include "../../include/utils/jwt_utils.h"
#include "../../include/utils/checkInput_utils.h"
#include "../../include/utils/macro_utils.h"

//---------------------------------------------------------------------------------------------
void login(int sockfd, const char *data) {
    CHECK_DB_CONN(sockfd, "LOGIN", conn);
    char *parsed_values[2];
    VALIDATE_INPUT(data, login_config, parsed_values, 2, "LOGIN", sockfd, conn);

    char *email = parsed_values[0], *password = parsed_values[1];

    // Check user credentials
    const char *query = "SELECT user_id, password FROM users WHERE email = $1";
    const char *params[1] = { email };
    PGresult *res = PQexecParams(conn, query, 1, NULL, params, NULL, NULL, 0);
    HANDLE_QUERY_RESULT(res, "LOGIN", "Invalid email or password", conn);

    if (PQntuples(res) == 0) {
        SEND_ERROR(sockfd, "LOGIN", 401, "Invalid email or password", conn, res);
        PQclear(res);
        free_parsed_values(parsed_values, 2);
        return;
    }

    const char *stored_hash = PQgetvalue(res, 0, 1);
    if (!verify_password(password, stored_hash)) {
        SEND_ERROR(sockfd, "LOGIN", 401, "Invalid email or password", conn, res);
        free_parsed_values(parsed_values, 2);
        return;
    }

    char *token = NULL;
    if (generate_jwt(PQgetvalue(res, 0, 0), config->SECRET_KEY, &token, conn) != 0) {
        SEND_ERROR(sockfd, "LOGIN", 500, "Failed to generate token", conn, res);
        free_parsed_values(parsed_values, 2);
        return;
    }

    char response[256 + 21];
    snprintf(response, sizeof(response), "LOGIN|SUCCESS|200|%s", token);
    send(sockfd, response, strlen(response), 0);

    free(token);
    PQclear(res);
    disconnect_db(conn);
    free_parsed_values(parsed_values, 2);
}

//---------------------------------------------------------------------------------------------
void signup(int sockfd, const char *data) { 
    CHECK_DB_CONN(sockfd, "SIGNUP", conn);
    char *parsed_values[3];
    VALIDATE_INPUT(data, signup_config, parsed_values, 3, "SIGNUP", sockfd, conn);

    char *email = parsed_values[0], *username = parsed_values[1], *password = parsed_values[2];

    char hashed_password[crypto_pwhash_STRBYTES];
    if (hash_password(password, hashed_password) != 0) {
        SEND_ERROR(sockfd, "SIGNUP", 500, "Mã hóa mật khẩu thất bại", conn, NULL);
    }

    // Insert user data and get user_id
    const char *query = "INSERT INTO users (email, username, password) VALUES ($1, $2, $3) RETURNING user_id";
    const char *paramValues[3] = { email, username, hashed_password };
    PGresult *res = PQexecParams(conn, query, 3, NULL, paramValues, NULL, NULL, 0);

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        const char *error_message = (strcmp(PQresultErrorField(res, PG_DIAG_SQLSTATE), "23505") == 0)
            ? "Email existed"
            : "Register failed";
        SEND_ERROR(sockfd, "SIGNUP", (strcmp(error_message, "Email existed") == 0) ? 409 : 500, error_message, conn, res);
    }

    // Lấy user_id từ kết quả trả về
    char *user_id = PQgetvalue(res, 0, 0);

    // Tạo thư mục uploads/user_id
    char folder_path[256];
    snprintf(folder_path, sizeof(folder_path), "uploads/%s", user_id);

    // Create the user-specific directory if it doesn't exist
    struct stat st = {0};
    if (stat(folder_path, &st) == -1) {
        if (mkdir(folder_path, 0777) != 0) {
            SEND_ERROR(sockfd, "SIGNUP", 500, "Không thể tạo thư mục cho người dùng", conn, res);
        }
    }

    send(sockfd, "SIGNUP|SUCCESS|200", 20, 0);

    // Dọn dẹp tài nguyên
    PQclear(res);
    disconnect_db(conn);
    free_parsed_values(parsed_values, 3);
}


//---------------------------------------------------------------------------------------------
void logout(int sockfd, const char *data) {
    CHECK_DB_CONN(sockfd, "LOGOUT", conn);
    char *parsed_values[1];
    VALIDATE_INPUT(data, logout_config, parsed_values, 1, "LOGOUT", sockfd, conn);

    char *token = parsed_values[0];

    if (!verify_jwt(token, config->SECRET_KEY, conn)) {
        SEND_ERROR(sockfd, "LOGOUT", 401, "Unauthorized", conn, NULL);
    }

    if (delete_jwt(token, conn) != 0) {
        SEND_ERROR(sockfd, "LOGOUT", 500, "Error deleting token", conn, NULL);
    }

    send(sockfd, "LOGOUT|SUCCESS|200", 18, 0);

    disconnect_db(conn);
    free(parsed_values[0]);
}


//---------------------------------------------------------------------------------------------
void change_password(int sockfd, const char *data) {
    CHECK_DB_CONN(sockfd, "CHANGE_PASSWORD", conn);
    char *parsed_values[3];
    VALIDATE_INPUT(data, change_password_config, parsed_values, 3, "CHANGE_PASSWORD", sockfd, conn);

    char *old_password = parsed_values[0], *new_password = parsed_values[1], *token = parsed_values[2];

    int user_id = verify_jwt(token, config->SECRET_KEY, conn);
    if (user_id == -1) {
        SEND_ERROR(sockfd, "CHANGE_PASSWORD", 401, "Unauthorized", conn, NULL);
    }

    const char *query = "SELECT password FROM users WHERE user_id = $1";
    char user_id_str[12];
    snprintf(user_id_str, sizeof(user_id_str), "%d", user_id);
    const char *params[1] = { user_id_str };
    PGresult *res = PQexecParams(conn, query, 1, NULL, params, NULL, NULL, 0);
    HANDLE_QUERY_RESULT(res, "CHANGE_PASSWORD", "Không tìm thấy người dùng", conn);

    const char *hashed_password = PQgetvalue(res, 0, 0);
    if (crypto_pwhash_str_verify(hashed_password, old_password, strlen(old_password)) != 0) {
        SEND_ERROR(sockfd, "CHANGE_PASSWORD", 403, "Mật khẩu không chính xác", conn, res);
    }
    PQclear(res);

    if (!strcmp(old_password, new_password)) {
        SEND_ERROR(sockfd, "CHANGE_PASSWORD", 400, "Mật khẩu mới không được giống mật khẩu cũ", conn, NULL);
    }

    char new_hashed_password[crypto_pwhash_STRBYTES];
    if (hash_password(new_password, new_hashed_password) != 0) {
        SEND_ERROR(sockfd, "CHANGE_PASSWORD", 500, "Không thể mã hóa mật khẩu mới", conn, NULL);
    }

    const char *update_query = "UPDATE users SET password = $1 WHERE user_id = $2";
    const char *update_params[] = { new_hashed_password, user_id_str };
    PGresult *update_res = PQexecParams(conn, update_query, 2, NULL, update_params, NULL, NULL, 0);

    if (PQresultStatus(update_res) != PGRES_COMMAND_OK) {
        SEND_ERROR(sockfd, "CHANGE_PASSWORD", 500, "Không thể thay đổi mật khẩu", conn, update_res);
    }

    send(sockfd, "CHANGE_PASSWORD|SUCCESS|200", 29, 0);

    PQclear(update_res);
    disconnect_db(conn);
    free_parsed_values(parsed_values, 3);
}


