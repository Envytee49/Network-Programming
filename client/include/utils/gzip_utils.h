// gzip_utils.h
#ifndef GZIP_UTILS_H
#define GZIP_UTILS_H

#include <stdio.h>

#define BUFFER_SIZE 1024

// Hàm giải nén gzip
int decompress_buffer(const char *input, int input_size, char *output);

void print_directory_tree(const char *encoded_data);

#endif // GZIP_UTILS_H
