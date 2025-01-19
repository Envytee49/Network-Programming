#ifndef INPUT_UTILS_H
#define INPUT_UTILS_H

#include <stddef.h>

// Hàm kiểm tra định dạng
int is_valid_int(const char *input);
int is_valid_text_no_space(const char *input);
int is_valid_date(const char *input);

// Hàm nhập dữ liệu
void input_int(char *prompt, char *buffer, size_t size);
void input_text_no_space(char *prompt, char *buffer, size_t size);
void input_text(char *prompt, char *buffer, size_t size);
void input_date(char *prompt, char *buffer, size_t size);


#endif 
