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

void check_archive_test(int fd)
{
    int ret = check_archive(fd);
    printf("check_archive() returned : %d\n\n", ret);
}

void exists_test(int fd, char *path)
{
    int ret = exists(fd, path);
    printf("exists() returned : %d\n\n", ret);
}

void is_x_test(int fd, char *path, char *type)
{
    int ret;
    if (strcmp(type, "symlink") == 0)
    {
        ret = is_symlink(fd, path);
        printf("is_symlink() returned : %d\n\n", ret);
    }
    else if (strcmp(type, "file") == 0)
    {
        ret = is_file(fd, path);
        printf("is_file returned : %d\n\n", ret);
    }
    else if (strcmp(type, "dir") == 0)
    {
        ret = is_dir(fd, path);
        printf("is_dir() returned : %d\n\n", ret);
    }
}

void list_test(int fd, char *path, size_t no_entries)
{
    char *entries[no_entries];
    for (int i = 0; i < no_entries; i++) entries[i] = malloc(100 * sizeof(char));
    
    int ret_list = list(fd, path, entries, &no_entries);

    printf("list() returned : %d\n", ret_list);
    printf("no_entries : %ld\n", no_entries);
    printf("The list : [ ");
    for (int i = 0; i < no_entries - 1; i++) printf("%s, ", entries[i]);
    printf("%s ]\n\n", entries[no_entries - 1]);
}


void read_file_test(int fd, char *path, size_t offset, size_t len)
{
    size_t copy_len = len;
    uint8_t *buffer = malloc(len * sizeof(char));
    int ret_read_file = read_file(fd, path, offset, buffer, &len);
    if (ret_read_file > 0)
    {
        if (len + ret_read_file != copy_len) printf("ERROR !!!\n");
    }
    printf("read_file() returned : %d\n", ret_read_file);
    printf("len : %ld\n", len);
    printf("The file :\n%s\n\n", (char *) buffer);
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

    // TEST : BEGIN

    check_archive_test(fd);
    exists_test(fd, "folder_test/");
    is_x_test(fd, "folder_test/", "file");
    is_x_test(fd, "folder_test/", "dir");
    is_x_test(fd, "folder_test/", "symlink");
    list_test(fd, "folder_test/", 100);
    read_file_test(fd, "folder_test/file1.txt", 0, 63);

    // TEST : END

    return EXIT_SUCCESS;
}