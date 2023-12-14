#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

#include "lib_tar.h"

void debug_dump(const uint8_t *bytes, size_t len)
{
    for (int i = 0; i < len;)
    {
        printf("%04x:  ", (int) i);
        for (int j = 0; j < 16 && i + j < len; j++) printf("%02x ", bytes[i + j]);
        printf("\t");
        for (int j = 0; j < 16 && i < len; j++, i++) printf("%c ", bytes[i]);
        printf("\n");
    }
}

void check_archive_test(int fd, int expected)
{
    int ret = check_archive(fd);
    if (expected != ret) printf("ERROR : check_archive()\nReturn %d instead of %d\n", ret, expected);
}

void exists_test(int fd, char *path, int expected)
{
    int ret = exists(fd, path);
    if (expected != ret) printf("ERROR : exists()\nReturn %d instead of %d\n[args : path = %s ]\n", ret, expected, path);
}

void is_x_test(int fd, char *path, char *type, int expected)
{
    int ret;
    char *function_name;
    if (strcmp(type, "symlink") == 0)
    {
        ret = is_symlink(fd, path);
        function_name = "is_symlink";
    }
    else if (strcmp(type, "file") == 0)
    {
        ret = is_file(fd, path);
        function_name = "is_file";
    }
    else if (strcmp(type, "dir") == 0)
    {
        ret = is_dir(fd, path);
        function_name = "is_dir";
    }
    if (expected != ret) printf("ERROR : %s()\nReturn %d instead of %d\n[args : path = %s ]\n", function_name, ret, expected, path);
}

int cmper_str(const void *a, const void *b)
{
    return strcmp(*(const char **)a, *(const char **)b);
}

void list_test(int fd, char *path, size_t no_entries, int expected_ret, int expected_no_entries, char *expected_entries[])
{
    size_t copy_no_entries = no_entries;
    char *entries[no_entries];
    for (int i = 0; i < no_entries; i++) entries[i] = calloc(sizeof(char), 100);
    int ret = list(fd, path, entries, &no_entries);

    int error = 1;
    if (expected_ret != ret) error = 0;
    if (expected_no_entries != no_entries) error = 0;

    qsort(expected_entries, expected_no_entries, sizeof(char *), cmper_str);
    qsort(entries, expected_no_entries, sizeof(char *), cmper_str);

    for (int i = 0; i < expected_no_entries; i++)
    {
        if (strcmp(expected_entries[i], entries[i]) != 0)
        {
            error = 0;
            break;
        }
    }
    
    if (error == 0)
    {
        char entries_str[100000] = "[ ";
        char expected_entries_str[100000] = "[ ";
        for (int i = 0; i < expected_no_entries - 1; i++)
        {
            strcat(expected_entries_str, expected_entries[i]);
            strcat(expected_entries_str, ", ");
            strcat(entries_str, entries[i]);
            strcat(entries_str, ", ");
        }
        if (expected_no_entries - 1 >= 0)
        {
            strcat(expected_entries_str, expected_entries[expected_no_entries - 1]);
            strcat(entries_str, entries[expected_no_entries - 1]);
        }
        strcat(expected_entries_str, "]");
        strcat(entries_str, "]");
        
        printf("ERROR : list() [args : path = %s , no_entries = %ld ]\n", path, copy_no_entries);
        printf("no_entries = %ld AND expected_no_entries = %d\n", no_entries, expected_no_entries);
        printf("ret = %d AND expected_ret = %d\n", ret, expected_ret);
        printf("entries = %s AND expected_entries = %s\n", entries_str, expected_entries_str);
    }

    for (int i = 0; i < no_entries; i++) free(entries[i]);
}


void read_file_test(int fd, char *path, size_t offset, size_t len, int expected_ret, int expected_len, char *expected_buffer)
{
    uint8_t *buffer = calloc(sizeof(char), len);
    int ret = read_file(fd, path, offset, buffer, &len);
    if (expected_ret != ret) printf("ERROR : read_file()\nReturn %d instead of %d\n[args : path = %s ]\n", ret, expected_ret, path);
    if (expected_len != len) printf("ERROR : read_file()\nlen = %ld instead of len = %d\n[args : path = %s ]\n", len, expected_len, path);
    if (strcmp(expected_buffer, (char *) buffer) != 0) printf("ERROR : read_file()\nbuffer = %s instead of buffer = %s\n[args : path = %s ]\n", buffer, expected_buffer, path);
    free(buffer);
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("Usage: %s tar_file\n", argv[0]);
        return EXIT_FAILURE;
    }

    int fd = open(argv[1] , O_RDONLY);
    if (fd == -1)
    {
        perror("open(tar_file)");
        return EXIT_FAILURE;
    }

    // *** check_archive_test() : BEGIN ***
    check_archive_test(fd, 22);
    // *** check_archive_test() : END ***


    // *** exists_test() : BEGIN ***
    exists_test(fd, "folder1/file1.txt", 1);
    exists_test(fd, "folder1/subfolder1_1/file1_2.txt", 1);
    exists_test(fd, "folder3/", 1);
    exists_test(fd, "folder2/subfolder2_1/", 1);
    exists_test(fd, "symlink1", 1);
    exists_test(fd, "folder2/symlink3", 1);

    exists_test(fd, "folder2/subfolder2_1/inconnue.txt", 0);
    exists_test(fd, "doesnt_exist.txt", 0);
    exists_test(fd, "doesnt_exist_symlink", 0);
    exists_test(fd, "folder_doesnt_exist/", 0);
    // *** exists_test() : END ***


    // *** is_x_test() : BEGIN ***
    is_x_test(fd, "folder1/subfolder1_1/file1_2.txt", "file", 1);
    is_x_test(fd, "folder1/file1.txt", "file", 1);
    is_x_test(fd, "folder3/file3_1.txt", "file", 1);
    is_x_test(fd, "folder1/subfolder1_1/", "file", 0);
    is_x_test(fd, "symlink1", "file", 0);
    is_x_test(fd, "folder3/", "file", 0);

    is_x_test(fd, "folder1/subfolder1_1/", "dir", 1);
    is_x_test(fd, "folder1/", "dir", 1);
    is_x_test(fd, "folder3/", "dir", 1);
    is_x_test(fd, "symlink1", "dir", 0);
    is_x_test(fd, "folder1/file1.txt", "dir", 0);
    is_x_test(fd, "folder2/symlink4", "dir", 0);

    is_x_test(fd, "folder1/symlink2", "symlink", 1);
    is_x_test(fd, "symlink1", "symlink", 1);
    is_x_test(fd, "folder2/symlink4", "symlink", 1);
    is_x_test(fd, "folder1/subfolder1_1/", "symlink", 0);
    is_x_test(fd, "folder3/", "symlink", 0);
    is_x_test(fd, "folder1/subfolder1_1/file1_2.txt", "symlink", 0);
    // *** is_x_test() : END ***


    // *** list_test() : BEGIN ***
    // fd - path - no_entries - expected_ret - expected_no_entries - expected_entries
    char *expected_entries_1[] = {"folder1/subfolder1_1/", "folder1/file1.txt", "folder1/symlink2"};
    list_test(fd, "folder1/", 3, 3, 3, expected_entries_1);

    char *expected_entries_2[] = {"folder3/file3_1.txt", "folder3/symlink5"};
    list_test(fd, "folder3/", 2, 2, 2, expected_entries_2);

    char *expected_entries_3[] = {"folder2/subfolder2_1/", "folder2/subfolder2_2/", "folder2/symlink3", "folder2/symlink4"};
    list_test(fd, "folder2/", 4, 4, 4, expected_entries_3);

    char *expected_entries_4[] = {""};
    list_test(fd, "doesnt_exist/", 2, 0, 0, expected_entries_4);

    char *expected_entries_5[] = {""};
    list_test(fd, "folder3/file3_1.txt", 2, 0, 0, expected_entries_5);

    char *expected_entries_6[] = {"folder1/subfolder1_1/file1_1.txt", "folder1/subfolder1_1/file1_2.txt"};
    list_test(fd, "symlink1", 2, 2, 2, expected_entries_6);

    char *expected_entries_7[] = {""};
    list_test(fd, "folder1/symlink2", 2, 0, 0, expected_entries_7);

    char *expected_entries_8[] = {"folder2/subfolder2_2/file2_2_1.txt"};
    list_test(fd, "folder2/symlink3", 1, 1, 1, expected_entries_8);

    char *expected_entries_9[] = {"folder4/text1.txt", "folder4/text2.txt", "folder4/text3.txt"};
    list_test(fd, "folder4/", 3, 3, 3, expected_entries_9);

    char *expected_entries_10[] = {""};
    list_test(fd, "folder_empty", 2, 0, 0, expected_entries_10);

    char *expected_entries_11[] = {"folder4/text1.txt", "folder4/text2.txt", "folder4/text3.txt"};
    list_test(fd, "folder3/symlink5", 3, 3, 3, expected_entries_11);
    // *** list_test() : END ***


    // *** read_file_test() : BEGIN ***
    // fd - path - offset - len - expected_ret - expected_len - expected_buffer
    read_file_test(fd, "folder1/subfolder1_1/file1_2.txt", 0, 1000, 0, 342, "My fellow citizens, let us embrace the dawn of a new era.\nIn the symphony of democracy, every note contributes to the melody of progress.\nWe must be the architects of our shared destiny, champions of justice, and stewards of liberty.\nTogether, we navigate the uncharted waters of the future, anchored by the values that define us as a people.");
    read_file_test(fd, "folder1/subfolder1_1/file1_2.txt", 0, 342, 0, 342, "My fellow citizens, let us embrace the dawn of a new era.\nIn the symphony of democracy, every note contributes to the melody of progress.\nWe must be the architects of our shared destiny, champions of justice, and stewards of liberty.\nTogether, we navigate the uncharted waters of the future, anchored by the values that define us as a people.");
    read_file_test(fd, "folder1/subfolder1_1/file1_2.txt", 0, 76, 266, 76, "My fellow citizens, let us embrace the dawn of a new era.\nIn the symphony of");
    read_file_test(fd, "folder1/subfolder1_1/file1_2.txt", 100, 242, 0, 242, "ontributes to the melody of progress.\nWe must be the architects of our shared destiny, champions of justice, and stewards of liberty.\nTogether, we navigate the uncharted waters of the future, anchored by the values that define us as a people.");
    read_file_test(fd, "folder1/subfolder1_1/file1_2.txt", 341, 1, 0, 1, ".");

    read_file_test(fd, "folder1/subfolder1_1/file1_2.txt", 342, 0, -2, 0, "");
    read_file_test(fd, "folder1/subfolder1_1/file1_2.txt", -1, 0, -2, 0, "");

    read_file_test(fd, "folder1/subfolder1_1/doesnt_exist.txt", 0, 1000, -1, 0, "");
    read_file_test(fd, "folder1/", 0, 1000, -1, 0, "");

    read_file_test(fd, "folder1/symlink2", 0, 1000, 0, 528, "Citizens and dreamers alike, let our aspirations soar higher than the tallest peaks.\nIn the grand tapestry of human endeavor, each thread is a story waiting to be told.\nLet our collective narrative be one of resilience, compassion, and boundless ambition.\nTogether, we paint the canvas of progress, guided by the enduring principles that define our shared humanity.\nAs we face the challenges of tomorrow, let us embrace the promise of a brighter, interconnected world, where the dreams of today become the realities of tomorrow.");
    // *** read_file_test() : END ***     folder3/file3_1.txt

    return EXIT_SUCCESS;
}