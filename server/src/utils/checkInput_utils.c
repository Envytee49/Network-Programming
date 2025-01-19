#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <ctype.h>
#include "../../include/utils/checkInput_utils.h"

// ==================================================================================
bool validate_input_dynamic(const char *data, const InputFieldConfig configs[], size_t field_count, char **parsed_values, bool debug) {
    const char *current = data;

    for (size_t i = 0; i < field_count; i++) {
        parsed_values[i] = NULL;
    }

    if (debug) printf("\n-----------------------------------------\n");
    if (debug) log_info("Begin Parsing Input");
    if (debug) log_info("Input: ");
    if (debug) printf("'%s'\n", data);
    if (debug) printf("Expected field count: %ld\n", field_count);

    for (size_t i = 0; i < field_count; i++) {
        if (debug) printf("Parsing field %zu: Current pointer -> '%s'\n", i, current);

        char buffer[512];
        size_t len = configs[i].max_length;

        // Kiểm tra nếu dấu phân cách '|' tồn tại và phân tích giá trị
        int result = sscanf(current, "%511[^|]", buffer);
        if (result != 1) {
            if (debug) {
                printf("ERROR: Unable to parse field %zu. Expected format: 'field|...'\n", i);
                log_error("Unable to parse field");
            }
            free_parsed_values(parsed_values, i);
            return false;
        }

        if (debug) printf("Parsed value for field %zu: '%s'\n", i, buffer);

        // Kiểm tra độ dài của giá trị
        if (strlen(buffer) > len) {
            if (debug) {
                printf("ERROR: Field %zu exceeds max length (%zu). Value: '%s'\n", i, len, buffer);
                log_error("Field exceeds max length");
            }
            free_parsed_values(parsed_values, i);
            return false;
        }

        // Gọi hàm validate của trường (nếu có)
        if (configs[i].validate && !configs[i].validate(buffer)) {
            if (debug) {
                printf("ERROR: Validation failed for field %zu. Value: '%s'\n", i, buffer);
                log_error("Invalid value for field");
            }
            free_parsed_values(parsed_values, i);
            return false;
        }

        // Sao chép giá trị vào parsed_values
        parsed_values[i] = strdup(buffer);
        if (parsed_values[i] == NULL) {
            if (debug) {
                printf("ERROR: Memory allocation failed for field %zu. Value: '%s'\n", i, buffer);
                log_error("Memory allocation failed");
            }
            free_parsed_values(parsed_values, i);
            return false;
        }

        // Di chuyển con trỏ đến trường tiếp theo (bỏ qua '|')
        current += strlen(buffer);

        // Kiểm tra nếu còn dấu phân cách '|' thì bỏ qua nó
        if (*current == '|') {
            current++; // Di chuyển qua dấu '|'
        } else if (i < field_count - 1) {
            if (debug) {
                printf("ERROR: Missing delimiter '|' after field %zu\n", i);
                log_error("Missing delimiter '|' after field");
            }
            free_parsed_values(parsed_values, field_count);
            return false;
        }
    }

    // Kiểm tra nếu còn dư dữ liệu sau khi parse
    if (*current != '\0') {
        if (debug) {
            printf("ERROR: Extra data found after parsing. Remaining: '%s'\n", current);
            log_error("Extra data found");
        }
        free_parsed_values(parsed_values, field_count);
        return false;
    }

    if (debug) {
        log_info("Parsing Completed Successfully");
        printf("-----------------------------------------\n");
    }
    return true;
}


// ==================================================================================
void log_info(const char *msg) {
    printf("INFO: %s\n", msg);  
}

void log_error(const char *msg) {
    printf("ERROR: %s\n", msg); 
    printf("-----------------------------------------\n"); 
}

// ==================================================================================
bool is_valid_email(const char* email) {
    const char* pattern = "^[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\\.[A-Za-z]{2,}$";
    regex_t regex;
    if (regcomp(&regex, pattern, REG_EXTENDED | REG_NOSUB) != 0) {
        return false; 
    }
    bool match = regexec(&regex, email, 0, NULL, 0) == 0;
    regfree(&regex);
    return match;
}

bool is_valid_username(const char* username) {
    if (strlen(username) < 5) {
        return false;
    }

    // Kiểm tra từng ký tự xem có phải là chữ cái hay không
    for (size_t i = 0; i < strlen(username); i++) {
        if (!isalpha(username[i]) && username[i] != ' ') {
            return false;
        }
    }

    return true;
}

bool is_valid_password(const char* password) {
    if (strlen(password) < 8) {
        return false;
    }

    // Kiểm tra từng ký tự trong mật khẩu xem có ký tự đặc biệt không
    for (size_t i = 0; i < strlen(password); i++) {
        if (ispunct(password[i])) {
            return false;
        }
    }

    return true;
}

bool is_valid_number(const char *value) {
    if (value == NULL || *value == '\0') return false;

    // Kiểm tra từng ký tự
    for (const char *c = value; *c != '\0'; c++) {
        if (!isdigit(*c)) return false; // Không phải số
    }
    return true;
}

// ==================================================================================
void free_parsed_values(char **parsed_values, size_t count) {
    for (size_t i = 0; i < count; i++) {
        if (parsed_values[i] != NULL) {
            free(parsed_values[i]);
            parsed_values[i] = NULL; 
        }
    }
}

// ==================================================================================

InputFieldConfig signup_config[] = {
    {"email", 254, is_valid_email},
    {"username", 64, is_valid_username},
    {"password", 128, is_valid_password}
};

InputFieldConfig login_config[] = {
    {"email", 254, is_valid_email},
    {"password", 128, is_valid_password}
};

InputFieldConfig logout_config[] = {
    {"token", 256, NULL}
};

InputFieldConfig change_password_config[] = {
    {"old_password", 128, is_valid_password},
    {"new_password", 128, is_valid_password},
    {"token", 256, NULL}
};

InputFieldConfig send_file_config[] = {
    {"file_path", 255, NULL},
    {"token", 256, NULL}
};

InputFieldConfig receive_file_config[] = {
    {"file_name", 255, NULL},
    {"N", 255, is_valid_number},
    {"token", 256, NULL}
};

InputFieldConfig send_folder_config[] = {
    {"folder_name", 255, NULL},
    {"token", 256, NULL}
};

InputFieldConfig receive_folder_config[] = {
    {"folder_name", 255, NULL},
    {"token", 256, NULL}
};

InputFieldConfig get_folders_config[] = {
    {"token", 256, NULL}
};

InputFieldConfig rename_config[] = {
    {"path", 255, NULL},
    {"name", 255, NULL},
    {"token", 256, NULL}
};

InputFieldConfig move_config[] = {
    {"path", 255, NULL},
    {"new_path", 255, NULL},
    {"token", 256, NULL}
};

InputFieldConfig delete_config[] = {
    {"path", 255, NULL},
    {"token", 256, NULL}
};

