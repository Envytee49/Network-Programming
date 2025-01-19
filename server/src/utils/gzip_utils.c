#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <zlib.h>

#define MAX_PATH_LENGTH 1024
#define BUFFER_SIZE 4096
#define CHUNK 1024

void encode_directory(const char *path, char *buffer) {
    DIR *dir = opendir(path);
    struct dirent *entry;
    char fullPath[MAX_PATH_LENGTH];
    struct stat entry_stat;

    if (!dir) {
        perror("Failed to open directory");
        return;
    }

    // Duyệt qua các mục trong thư mục
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        snprintf(fullPath, sizeof(fullPath), "%s/%s", path, entry->d_name);
        if (stat(fullPath, &entry_stat) == -1) {
            perror("Stat failed");
            continue;
        }

        // Thêm thông tin vào buffer dưới dạng "path|type"
        if (S_ISDIR(entry_stat.st_mode)) {
            snprintf(buffer + strlen(buffer), BUFFER_SIZE - strlen(buffer), "%s/|", entry->d_name);
        } else {
            snprintf(buffer + strlen(buffer), BUFFER_SIZE - strlen(buffer), "%s|", entry->d_name);
        }
    }

    closedir(dir);
}