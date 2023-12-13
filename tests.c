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
    if (expected != ret) printf("ERROR : exists()\nReturn %d instead of %d [args : path = %s ]\n", ret, expected, path);
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
    if (expected != ret) printf("ERROR : %s()\nReturn %d instead of %d [args : path = %s ]\n", function_name, ret, expected, path);
}

void list_test(int fd, char *path, size_t no_entries)
{
    // TODO
    char *entries[no_entries];
    for (int i = 0; i < no_entries; i++) entries[i] = malloc(100 * sizeof(char));
    
    int ret_list = list(fd, path, entries, &no_entries);

    printf("list() returned : %d\n", ret_list);
    printf("no_entries : %ld\n", no_entries);
    printf("The list : [ ");

    if (no_entries == 0) printf(" ]\n\n");
    else if (no_entries == 1) printf("%s ]\n\n", entries[0]);
    else
    {
        for (size_t i = 0; i < no_entries; i++) printf("%s, ", entries[i]);
        printf("%s ]\n\n", entries[no_entries - 1]);
    }
}


void read_file_test(int fd, char *path, size_t offset, size_t len, int expected_ret, int expected_len, int verbose)
{
    size_t copy_len = len;
    uint8_t *buffer = malloc(len * sizeof(char));
    int ret = read_file(fd, path, offset, buffer, &len);
    if (expected_ret != ret) printf("ERROR : read_file()\nReturn %d instead of %d [args : path = %s ]\n", ret, expected_ret, path);
    if (expected_len != len) printf("ERROR : read_file()\nlen = %d instead of len = %d [args : path = %s ]\n", len, expected_len, path);

    if (verbose) printf("The file :\n%s\n\n", (char *) buffer);
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
    check_archive_test(fd, 15);
    // *** check_archive_test() : END ***


    // *** exists_test() : BEGIN ***
    exists_test(fd, "folder1/file1.txt", 1);
    exists_test(fd, "folder1/subfolder1_1/file1_2.txt", 1);
    exists_test(fd, "folder3/", 1);
    exists_test(fd, "folder2/subfolder2_1/", 1);
    exists_test(fd, "symlink4", 1);
    exists_test(fd, "folder2/symlink3", 1);

    exists_test(fd, "folder2/subfolder2_1/inconnue.txt", 0);
    exists_test(fd, "doesnt_exist.txt", 0);
    exists_test(fd, "doesnt_exist_symlink", 0);
    exists_test(fd, "folder_doesnt_exist/", 0);
    // *** exists_test() : END ***


    // TODO !


    // *** is_x_test() : BEGIN ***
    is_x_test(fd, "", "file", 1);
    is_x_test(fd, "", "file", 1);
    is_x_test(fd, "", "file", 1);
    is_x_test(fd, "", "file", 0);
    is_x_test(fd, "", "file", 0);
    is_x_test(fd, "", "file", 0);

    is_x_test(fd, "", "dir", 1);
    is_x_test(fd, "", "dir", 1);
    is_x_test(fd, "", "dir", 1);
    is_x_test(fd, "", "dir", 0);
    is_x_test(fd, "", "dir", 0);
    is_x_test(fd, "", "dir", 0);

    is_x_test(fd, "", "symlink", 1);
    is_x_test(fd, "", "symlink", 1);
    is_x_test(fd, "", "symlink", 1);
    is_x_test(fd, "", "symlink", 0);
    is_x_test(fd, "", "symlink", 0);
    is_x_test(fd, "", "symlink", 0);
    // *** is_x_test() : END ***


    // *** list_test() : BEGIN ***
    list_test(fd, "", 100);
    // *** list_test() : END ***


    // *** read_file_test() : BEGIN ***
    read_file_test(fd, "", 0, 63, 0, 0, 0);
    // *** read_file_test() : END ***

    return EXIT_SUCCESS;
}