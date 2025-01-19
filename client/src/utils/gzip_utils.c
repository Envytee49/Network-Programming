#include <zlib.h>
#include <string.h>
#include <stdio.h>
#include "utils/gzip_utils.h"

#define MAX_PATH_LENGTH 256 

void decode_directory(const char *encoded, int level) {
    char path[MAX_PATH_LENGTH];
    int i = 0;
    int path_len = 0;

    while (encoded[i] != '\0') {
        // Đọc một mục trong thư mục
        path_len = 0;
        while (encoded[i] != '|' && encoded[i] != '\0') {
            path[path_len++] = encoded[i++];
        }
        path[path_len] = '\0'; // Kết thúc chuỗi

        // Xử lý thư mục hoặc tệp
        if (encoded[i] == '|') {
            i++; // Bỏ qua ký tự '|'

            // Kiểm tra xem là thư mục hay tệp
            if (path[path_len - 1] == '/') {
                // Nếu là thư mục
                printf("%*s└── %s\n", level * 4, "", path);
                decode_directory(path, level + 1); // Đệ quy để giải mã thư mục con
            } else {
                // Nếu là tệp
                printf("%*s├── %s\n", level * 4, "", path);
            }
        }
    }
}