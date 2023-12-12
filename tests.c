#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

#include "lib_tar.h"

/*
You are free to use this file to write tests for your implementation
*/

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

void error(char *msg)
{
    perror(msg);
    exit(1);
}

void true_tests(int fd)
{
    int ret_check_archive = check_archive(fd);
    printf("check_archive returned : %d\n\n", ret_check_archive);

    char *path_exists = "folder_test/";
    int ret_exists = exists(fd, path_exists);
    printf("exists returned : %d\n\n", ret_exists);

    char *path_is_dir = "folder_test/";
    int ret_is_dir = is_dir(fd, path_is_dir);
    printf("Directory returned : %d\n\n", ret_is_dir);
    
    char *path_is_file = "folder_test/file1.txt";
    int ret_is_file = is_file(fd, path_is_file);
    printf("File returned : %d\n\n", ret_is_file);

    char *path_is_symlink = "folder_test/";
    int ret_is_symlink = is_symlink(fd, path_is_symlink);
    printf("Symlink returned : %d\n\n", ret_is_symlink);

    char *path_list = "folder_test/";
    size_t no_entries_list = 100;
    char *entries_list[no_entries_list];
    for (int i = 0; i < no_entries_list; i++) entries_list[i] = malloc(100 * sizeof(char));
    
    int ret_list = list(fd, path_list, entries_list, &no_entries_list);
    printf("list returned : %d\n\n", ret_list);
    if (ret_list == 0) error("list");

    printf("The list :\n");
    for (int i = 0; i < no_entries_list; i++)
    {
        printf("%s\n", entries_list[i]);
    }

    size_t copy_len_read_file = 63;
    size_t len_read_file  = 63;
    uint8_t *buffer_read_file = malloc(len_read_file * sizeof(char));
    char *path_read_file = "folder_test/file1.txt";
    size_t offset = 0;
    int ret_read_file = read_file(fd, path_read_file, offset, buffer_read_file, &len_read_file);
    if (ret_read_file > 0)
    {
        if (len_read_file + ret_read_file != copy_len_read_file) printf("ERROR !!!\n");
    }
    
    printf("len_read_file : %ld\n", len_read_file);
    printf("Read_file returned : %d\n\n", ret_read_file);
    printf("The file :\n%s\n\n", (char *) buffer_read_file);

}

void false_tests(int fd)
{
    // TODO
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("Usage: %s tar_file\n", argv[0]);
        return -1;
    }

    int fd = open(argv[1] , O_RDONLY);
    if (fd == -1)
    {
        perror("open(tar_file)");
        return -1;
    }

    true_tests(fd);
    false_tests(fd);

    return 0;
}