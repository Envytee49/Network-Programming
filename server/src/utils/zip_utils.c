// #include <zip.h>
// #include <dirent.h>
// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>

// int zip_folder_to_file(const char *folder_path, const char *zip_file_path) {
//     struct zip_t *zip = zip_open(zip_file_path, ZIP_DEFAULT_COMPRESSION_LEVEL, 'w');
//     if (!zip) {
//         fprintf(stderr, "Không thể mở file zip: %s\n", zip_file_path);
//         return -1;
//     }

//     DIR *dir = opendir(folder_path);
//     if (!dir) {
//         fprintf(stderr, "Không thể mở thư mục: %s\n", folder_path);
//         zip_close(zip);
//         return -1;
//     }

//     struct dirent *entry;
//     while ((entry = readdir(dir)) != NULL) {
//         if (entry->d_type == DT_DIR) {
//             // Bỏ qua các thư mục "." và ".."
//             if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
//                 continue;
//             }

//             // Đệ quy nén thư mục con
//             char subdir_path[1024];
//             snprintf(subdir_path, sizeof(subdir_path), "%s/%s", folder_path, entry->d_name);
//             zip_folder_to_file(subdir_path, zip_file_path);  // Gọi đệ quy
//         } else if (entry->d_type == DT_REG) {
//             // Nếu là file, thêm vào zip
//             char file_path[1024];
//             snprintf(file_path, sizeof(file_path), "%s/%s", folder_path, entry->d_name);
//             if (zip_entry_open(zip, entry->d_name) == 0) {
//                 FILE *file = fopen(file_path, "rb");
//                 if (!file) {
//                     fprintf(stderr, "Không thể mở file: %s\n", file_path);
//                     continue;
//                 }

//                 char buffer[1024];
//                 size_t bytes_read;
//                 while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0) {
//                     zip_entry_write(zip, buffer, bytes_read);
//                 }

//                 fclose(file);
//                 zip_entry_close(zip);
//             }
//         }
//     }

//     closedir(dir);
//     zip_close(zip);
//     return 0;
// }
