#ifndef CHECK_INPUT_UTILS_H
#define CHECK_INPUT_UTILS_H

#include <stdbool.h>
#include <stddef.h>

// ============================= Định nghĩa kiểu dữ liệu ============================
typedef bool (*ValidationFunc)(const char*);

typedef struct {
    const char *field_name;             // Tên trường dữ liệu
    size_t max_length;                  // Kích thước tối đa của trường dữ liệu
    bool (*validate)(const char *);     // Hàm kiểm tra tính hợp lệ của dữ liệu
} InputFieldConfig;

// ============================== Hàm kiểm tra đầu vào ==============================
/**
 * Hàm kiểm tra chuỗi đầu vào theo cấu hình trường.
 * 
 * @param data          Chuỗi đầu vào dạng "field1|field2|..."
 * @param configs       Mảng cấu hình của các trường
 * @param field_count   Số lượng trường cần kiểm tra
 * @param parsed_values Mảng để lưu giá trị đã phân tích cú pháp (cần được giải phóng sau khi sử dụng)
 * @return              true nếu đầu vào hợp lệ, ngược lại false
 */

bool validate_input_dynamic(const char *data, const InputFieldConfig configs[], size_t field_count, char **parsed_values, bool debug);

// ========================= Hàm xử lý việc ghi log thông báo =======================
void log_info(const char *msg);
void log_error(const char *msg);

// ========================= Hàm kiểm tra từng trường cụ thể ========================
bool is_valid_email(const char *email);
bool is_valid_username(const char *username);
bool is_valid_password(const char *password);
bool is_valid_number(const char *value);

// ========================= Giải phóng giá trị trong parser ========================
/**
 * Giải phóng bộ nhớ của các giá trị đã phân tích.
 * 
 * @param parsed_values  Mảng chứa các giá trị đã phân tích.
 * @param count          Số lượng các giá trị cần giải phóng.
 */
void free_parsed_values(char **parsed_values, size_t count);

// =========================== Cấu hình cho từng hành động ==========================
extern InputFieldConfig signup_config[];         
extern InputFieldConfig login_config[];             
extern InputFieldConfig logout_config[];           
extern InputFieldConfig change_password_config[];   
   
extern InputFieldConfig send_file_config[];   
extern InputFieldConfig receive_file_config[];   
extern InputFieldConfig send_folder_config[];
extern InputFieldConfig receive_folder_config[];
extern InputFieldConfig get_folders_config[];

extern InputFieldConfig rename_config[];
extern InputFieldConfig move_config[];
extern InputFieldConfig delete_config[];

#endif