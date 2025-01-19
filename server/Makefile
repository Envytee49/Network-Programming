CC = gcc
CFLAGS = -Wall -Wextra -g
INCLUDES = -Iinclude
LIBS = -lpq -ljwt -lconfig -lsodium -lz -lzip

SRCS = src/core/auth.c src/core/file.c src/core/folder.c src/core/managePaths.c src/database.c src/handler.c src/server.c src/config.c src/utils/jwt_utils.c  src/utils/checkInput_utils.c 
OBJS = $(patsubst src/%.c, obj/%.o, $(SRCS))  
TARGET = server

# Quy trình tạo thư mục cho các tệp .o
obj/%.o: src/%.c
	@mkdir -p $(dir $@)  # Tạo thư mục đích nếu chưa tồn tại
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(TARGET) $(OBJS) $(LIBS)

run: $(TARGET)
	./$(TARGET)

# # Mục tiêu cài đặt các thư viện phụ thuộc
# install:
# 	@echo "Cài đặt các thư viện phụ thuộc..."
# 	# Cài đặt các thư viện từ hệ thống (dành cho Ubuntu/Debian)
# 	sudo apt-get update
# 	sudo apt-get install -y libpq-dev libjwt-dev libsodium-dev zlib1g-dev
# 	# Cài đặt các thư viện khác nếu cần

clean:
	rm -rf obj $(TARGET)
