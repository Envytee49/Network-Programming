CC = gcc
CFLAGS = -Wall -Wextra -g
INCLUDES = -Iinclude
LIBS = -lpq -ljwt -lconfig -lsodium -lz

SRCS = src/client.c src/user_actions.c src/core/auth.c src/core/file.c src/core/folder.c src/core/managePaths.c src/utils/network_utils.c src/utils/input_utils.c
OBJS = $(patsubst src/%.c, obj/%.o, $(SRCS))
TARGET = client

# Tạo thư mục obj/utils nếu chưa có
obj/%.o: src/%.c
	@mkdir -p $(dir $@)  # Tạo thư mục cho từng file object
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(TARGET) $(OBJS) $(LIBS)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -rf obj $(TARGET)

rebuild: clean $(TARGET)

.PHONY: all clean rebuild
