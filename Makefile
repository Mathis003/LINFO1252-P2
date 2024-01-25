CC = gcc
CFLAGS = -g -Wall -Werror -Wextra

SRC_DIR = src
BIN_DIR = bin

SOURCES = $(wildcard $(SRC_DIR)/*.c)
OBJECTS = $(patsubst $(SRC_DIR)/%.c, $(BIN_DIR)/%.o, $(SOURCES))
EXECUTABLE = my_program

all: build run

run:
	@./$(EXECUTABLE)

build: $(BIN_DIR) $(EXECUTABLE) tar

$(EXECUTABLE): $(OBJECTS)
	@$(CC) $(CFLAGS) $^ -o $@

$(BIN_DIR)/%.o: $(SRC_DIR)/%.c
	@$(CC) $(CFLAGS) -c $< -o $@

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

tar:
	@tar --posix --pax-option delete=".*" --pax-option delete="*time*" --no-xattrs --no-acl --no-selinux -c archive_test/folder1 archive_test/folder2 archive_test/folder3 archive_test/folder4 archive_test/symlink_multi archive_test/symlink1 > TAR_archive_test.tar

.PHONY: clean submit

clean:
	@rm -f $(EXECUTABLE) soumission.tar TAR_archive_test.tar
	@rm -r $(BIN_DIR)

submit: all
	@tar --posix --pax-option delete=".*" --pax-option delete="*time*" --no-xattrs --no-acl --no-selinux -c *.h *.c Makefile > soumission.tar