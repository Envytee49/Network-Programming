#ifndef MACRO_UTILS_H
#define MACRO_UTILS_H

// ----------------------------------------------------------------------------------------
// Macro để gửi lỗi cho client và đóng kết nối DB
// Tạo một thông báo lỗi và gửi nó đến client. Sau đó, giải phóng kết quả truy vấn 
// và ngắt kết nối với cơ sở dữ liệu.
// ----------------------------------------------------------------------------------------
#define SEND_ERROR(sockfd, request_type, code, message, conn, res) \
    do { \
        char response[1024]; \
        snprintf(response, sizeof(response), "%s|ERROR|%d|%s", request_type, code, message); \
        send(sockfd, response, strlen(response), 0); /* Gửi thông báo lỗi tới client */      \
        PQclear(res);                                /* Giải phóng kết quả truy vấn */       \
        disconnect_db(conn);                         /* Ngắt kết nối với DB */               \
        return;                                                                              \
    } while (0)

// ----------------------------------------------------------------------------------------
// Macro để kiểm tra kết nối DB
// Kiểm tra kết nối với cơ sở dữ liệu. Nếu kết nối không thành công, thoát khỏi hàm.
// ----------------------------------------------------------------------------------------
#define CHECK_DB_CONN(sockfd, request_type, conn) \
    PGconn* conn = check_db_connect(sockfd, request_type); \
    if (!conn) return; /* Nếu không thể kết nối DB, trả về */

// ----------------------------------------------------------------------------------------
// Macro để xử lý kết quả truy vấn DB
// Kiểm tra trạng thái của kết quả truy vấn. Nếu không thành công, gửi lỗi và ngắt kết nối.
// ----------------------------------------------------------------------------------------
#define HANDLE_QUERY_RESULT(res, query_type, error_msg, conn)       \
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {                   \
        SEND_ERROR(sockfd, query_type, 404, error_msg, conn, res);  \
    }

// ----------------------------------------------------------------------------------------
// Macro để kiểm tra và xác thực đầu vào
// Xác minh rằng dữ liệu đầu vào hợp lệ và đầy đủ theo cấu hình. Nếu không, gửi lỗi và ngắt kết nối.
// ----------------------------------------------------------------------------------------
#define VALIDATE_INPUT(data, configs, parsed_values, field_count, request_type, sockfd, conn)                   \
    if (!validate_input_dynamic(data, configs, field_count, parsed_values, false)) {                             \
        free_parsed_values(parsed_values, field_count);                                                         \
        char response[1024];                                                                                    \
        snprintf(response, sizeof(response), "%s|ERROR|400|Not enough data or invalid format", request_type); \
        send(sockfd, response, strlen(response), 0);                                                            \
        disconnect_db(conn);                                                                                    \
        return;                                                                                                 \
    }

#endif