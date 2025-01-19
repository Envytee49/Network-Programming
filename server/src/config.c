#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/config.h"

#define MAX_LINE_LENGTH 256

Config *config = NULL; 

void init_config() {
    if (config == NULL) {
        config = (Config *)malloc(sizeof(Config));  
        if (config == NULL) {
            perror("Không thể cấp phát bộ nhớ cho config");
            exit(EXIT_FAILURE);
        }
    }
}

int load_config(const char *filename, Config *config) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Không thể mở file cấu hình");
        return -1;
    }

    char line[MAX_LINE_LENGTH];

    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = 0;  

        if (strncmp(line, "DB_HOST", 7) == 0) {
            if (sscanf(line, "DB_HOST = \"%[^\"]\"", config->DB_HOST) != 1) {
                fprintf(stderr, "Lỗi đọc DB_HOST\n");
                fclose(file);
                return -1;
            }
        } else if (strncmp(line, "DB_PORT", 7) == 0) {
            if (sscanf(line, "DB_PORT = \"%[^\"]\"", config->DB_PORT) != 1) {
                fprintf(stderr, "Lỗi đọc DB_PORT\n");
                fclose(file);
                return -1;
            }
        } else if (strncmp(line, "DB_NAME", 7) == 0) {
            if (sscanf(line, "DB_NAME = \"%[^\"]\"", config->DB_NAME) != 1) {
                fprintf(stderr, "Lỗi đọc DB_NAME\n");
                fclose(file);
                return -1;
            }
        } else if (strncmp(line, "DB_USER", 7) == 0) {
            if (sscanf(line, "DB_USER = \"%[^\"]\"", config->DB_USER) != 1) {
                fprintf(stderr, "Lỗi đọc DB_USER\n");
                fclose(file);
                return -1;
            }
        } else if (strncmp(line, "DB_PASS", 7) == 0) {
            if (sscanf(line, "DB_PASS = \"%[^\"]\"", config->DB_PASS) != 1) {
                fprintf(stderr, "Lỗi đọc DB_PASS\n");
                fclose(file);
                return -1;
            }
        } else if (strncmp(line, "SECRET_KEY", 10) == 0) {
            if (sscanf(line, "SECRET_KEY = \"%[^\"]\"", config->SECRET_KEY) != 1) {
                fprintf(stderr, "Lỗi đọc SECRET_KEY\n");
                fclose(file);
                return -1;
            }
        } else if (strncmp(line, "PORT", 4) == 0) {
            if (sscanf(line, "PORT = %d", &config->PORT) != 1) {
                fprintf(stderr, "Lỗi đọc PORT\n");
                fclose(file);
                return -1;
            }
        }
    }

    fclose(file);
    
    return 0;
}

void free_config() {
    if (config != NULL) {
        free(config);
    }
}
