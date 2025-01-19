#ifndef GZIP_UTILS_H
#define GZIP_UTILS_H

#include <zlib.h>

#define MAX_PATH_LENGTH 1024    // Định nghĩa chiều dài tối đa của đường dẫn
#define CHUNK 1024              // Kích thước chunk cho nén
#define BUFFER_SIZE 1024

// Hàm mã hóa cấu trúc thư mục thành chuỗi văn bản
void encode_directory(const char *path, char *buffer) ;


#endif 
