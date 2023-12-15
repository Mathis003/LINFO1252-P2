#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "lib_tar.h"

/**
 * You are free to use this file to write tests for your implementation
 */

void debug_dump(const uint8_t *bytes, size_t len) {
    for (int i = 0; i < len;) {
        printf("%04x:  ", (int) i);

        for (int j = 0; j < 16 && i + j < len; j++) {
            printf("%02x ", bytes[i + j]);
        }
        printf("\t");
        for (int j = 0; j < 16 && i < len; j++, i++) {
            printf("%c ", bytes[i]);
        }
        printf("\n");
    }
}

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: %s tar_file\n", argv[0]);
        return -1;
    }

    int fd = open(argv[1] , O_RDONLY);
    if (fd == -1) {
        perror("open(tar_file)");
        return -1;
    }

    int ret = check_archive(fd);
    printf("check_archive returned %d\n\n", ret);

    ret = exists(fd, "lib_tar.c");
    printf("exists(lib_tar.c) returned %d\n", ret);
    ret = exists(fd, "fake.c");
    printf("exists(fake.c) returned %d\n", ret);
    ret = exists(fd, "test/");
    printf("exists(test/) returned %d\n", ret);
    ret = exists(fd, "test/test.txt");
    printf("exists(test/test.txt) returned %d\n\n", ret);
    ret = exists(fd, "test/test3/");
    printf("exists(test/test3/) returned %d\n\n", ret);

    ret = is_dir(fd, "test/");
    printf("is_dir(test/) returned %d\n", ret);
    ret = is_dir(fd, "lib_tar.h");
    printf("is_dir(lib_tar.h) returned %d\n", ret);
    ret = is_dir(fd, "test/test.txt");
    printf("is_dir(test/test.txt) returned %d\n\n", ret);

    ret = is_file(fd, "test/");
    printf("is_file(test/) returned %d\n", ret);
    ret = is_file(fd, "lib_tar.h");
    printf("is_file(lib_tar.h) returned %d\n", ret);
    ret = is_file(fd, "test/test.txt");
    printf("is_file(test/test.txt) returned %d\n\n", ret);

    ret = is_symlink(fd, "test/");
    printf("is_symlink(test/) returned %d\n", ret);
    ret = is_symlink(fd, "lib_tar.h");
    printf("is_symlink(lib_tar.h) returned %d\n", ret);
    ret = is_symlink(fd, "test/test.txt");
    printf("is_symlink(test/test.txt) returned %d\n", ret);
    ret = is_symlink(fd, "testlink");
    printf("is_symlink(testlink) returned %d\n\n", ret);

    size_t len = 50;
    unsigned char *str1 = (unsigned char*)malloc(len);
    ret = read_file(fd, "test/test.txt", 0, str1, &len);
    printf("contenu de test/test.txt : (longeur %ld) (return : %d)\n%s\n", len, ret, str1);

    len = 1000;
    unsigned char *str2 = (unsigned char*)malloc(len);
    ret = read_file(fd, "lib_tar.h", 500, str2, &len);
    printf("contenu de lib_tar.h : (longeur %ld) (return : %d)\n%s\n", len, ret, str2);

    len = 50;
    unsigned char *str3 = (unsigned char*)malloc(len);
    ret = read_file(fd, "testlink", 0, str3, &len);
    printf("contenu de testlink : (longeur %ld) (return : %d)\n%s\n", len, ret, str3);

    len = 50;
    unsigned char *str4 = (unsigned char*)malloc(len);
    ret = read_file(fd, "folderlinktest", 0, str4, &len);
    printf("contenu de folderlinktest : (longeur %ld) (return : %d)\n%s\n", len, ret, str4);

    size_t entry_num = 50;
    char **entries = (char**)malloc(50*sizeof(char*));
    for (int i = 0; i < entry_num; i++) entries[i] = (char*)malloc(100);
    list(fd, "folderlinktest", entries, &entry_num);
    printf("list folderlinktest : (entrynum = %ld)\n", entry_num);
    for (int i = 0; i < entry_num; i++) printf("%s\n", entries[i]);
    printf("\n");

    size_t entry_num2 = 50;
    char **entries2 = (char**)malloc(50*sizeof(char*));
    for (int i = 0; i < entry_num2; i++) entries2[i] = (char*)malloc(100);
    list(fd, "test/test3/", entries2, &entry_num2);
    printf("list test/test3/ : (entrynum = %ld)\n", entry_num2);
    for (int i = 0; i < entry_num2; i++) printf("%s\n", entries2[i]);
    printf("\n");

    size_t entry_num3 = 50;
    char **entries3 = (char**)malloc(50*sizeof(char*));
    for (int i = 0; i < entry_num3; i++) entries3[i] = (char*)malloc(100);
    list(fd, "empty/", entries3, &entry_num3);
    printf("list empty/ : (entrynum = %ld)\n", entry_num3);
    for (int i = 0; i < entry_num3; i++) printf("%s\n", entries3[i]);

    return 0;
}