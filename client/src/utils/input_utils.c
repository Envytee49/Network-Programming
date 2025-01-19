#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "utils/input_utils.h"

// Hàm kiểm tra chuỗi chỉ chứa số
int is_valid_int(const char *input) {
    for (size_t i = 0; i < strlen(input); i++) {
        if (!isdigit(input[i])) {
            return 0;  // Không phải số
        }
    }
    return 1;  
}

// Hàm kiểm tra chuỗi không chứa khoảng cách
int is_valid_text_no_space(const char *input) {
    for (size_t i = 0; i < strlen(input); i++) {
        if (isspace(input[i])) {
            return 0;  // Có khoảng cách
        }
    }
    return 1;  
}

// Hàm kiểm tra định dạng ngày (yyyy-MM-dd HH:mm)
int is_valid_date(const char *input) {
    int year, month, day, hour, minute;
    if (sscanf(input, "%4d-%2d-%2d %2d:%2d", &year, &month, &day, &hour, &minute) != 5) {
        return 0;  // Định dạng không đúng
    }
    if (month < 1 || month > 12 || day < 1 || day > 31 || hour < 0 || hour > 23 || minute < 0 || minute > 59) {
        return 0;  // Dữ liệu không hợp lệ
    }
    return 1;  
}

// Hàm nhập số nguyên
void input_int(char *prompt, char *buffer, size_t size) {
    do {
        printf("%s", prompt);
        if (fgets(buffer, size, stdin) == NULL) {
            printf("Lỗi: Đọc dữ liệu không thành công.\n");
            continue;
        }
        buffer[strcspn(buffer, "\n")] = 0;  // Xóa ký tự '\n'

        if (is_valid_int(buffer)) {
            break;  // Đầu vào hợp lệ
        } else {
            printf("Lỗi: Vui lòng nhập số nguyên hợp lệ.\n");
        }
    } while (1);
}

// Hàm nhập chuỗi không chứa khoảng cách
void input_text_no_space(char *prompt, char *buffer, size_t size) {
    do {
        printf("%s", prompt);
        if (fgets(buffer, size, stdin) == NULL) {
            printf("Lỗi: Đọc dữ liệu không thành công.\n");
            continue;
        }
        buffer[strcspn(buffer, "\n")] = 0;  // Xóa ký tự '\n'

        if (is_valid_text_no_space(buffer)) {
            break;  // Đầu vào hợp lệ
        } else {
            printf("Lỗi: Vui lòng nhập chuỗi không chứa khoảng cách.\n");
        }
    } while (1);
}

// Hàm nhập chuỗi có khoảng cách
void input_text(char *prompt, char *buffer, size_t size) {
    printf("%s", prompt);
    if (fgets(buffer, size, stdin) == NULL) {
        printf("Lỗi: Đọc dữ liệu không thành công.\n");
        return;
    }
    buffer[strcspn(buffer, "\n")] = 0;  // Xóa ký tự '\n'
}

// Hàm nhập ngày
void input_date(char *prompt, char *buffer, size_t size) {
    do {
        printf("%s", prompt);
        if (fgets(buffer, size, stdin) == NULL) {
            printf("Lỗi: Đọc dữ liệu không thành công.\n");
            continue;
        }
        buffer[strcspn(buffer, "\n")] = 0;  // Xóa ký tự '\n'

        if (is_valid_date(buffer)) {
            break;  // Đầu vào hợp lệ
        } else {
            printf("Lỗi: Vui lòng nhập ngày đúng định dạng (yyyy-MM-dd HH:mm).\n");
        }
    } while (1);
}


