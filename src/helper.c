#include "../headers/helper.h"

void get_info_header(tar_header_t header, int id)
{
    printf("header %d\n", id);
    printf("\theader.name : %s\n", header.name);
    printf("\theader.size : %ld\n", TAR_INT(header.size));
    printf("\theader.typeflag : %c\n", header.typeflag);
    printf("\theader.magic : %s\n", header.magic);
    printf("\theader.version : %s\n", header.version);
    printf("\theader.chksum : %ld\n\n", TAR_INT(header.chksum));
}


void skip_file_content(int tar_fd, tar_header_t header)
{
    int nb_blocks = TAR_INT(header.size) / HEADER_SIZE;
    if (TAR_INT(header.size) % HEADER_SIZE != 0) nb_blocks++;
    lseek(tar_fd, HEADER_SIZE * nb_blocks, SEEK_CUR);
}


void skip_dir(int tar_fd, tar_header_t *header)
{
    char *name_dir = (char *) malloc(100 * sizeof(char));
    memcpy(name_dir, header->name, 100 * sizeof(char));

    while (check_if_entry_folder(name_dir, header->name) == 1)
    {
        if (header->typeflag == REGTYPE || header->typeflag == AREGTYPE) skip_file_content(tar_fd, *header);
        if (read(tar_fd, header, HEADER_SIZE) <= 0 || header->name[0] == '\0') break;
    }

    free(name_dir);
}


int check_if_entry_folder(char *parent_dir, char *current_path)
{
    for (int i = 0; parent_dir[i] != '\0' ; i++)
    {
        if (current_path[i] == '\0' || parent_dir[i] != current_path[i]) return 0;
    }
    return 1;
}


char *parse_symlink(char *header_name, char *header_linkname)
{
    char *parsed_name = (char *) malloc(100 * sizeof(char));
    int len = strlen(header_name) - 1;
    // Get the len of the last '/'
    for (int i = len; i >= 0; i--)
    {
        if (header_name[i] != '/') len--;
        else break;
    }

    // Size max of a name : 100 characters
    if (strlen(header_linkname) + len + 1 >= 100) return parsed_name;
    
    if (len > 0)
    {
        memcpy(parsed_name, header_name, len + 1);
        strcat(parsed_name, header_linkname);
    }
    // No backslash in header_name
    else memcpy(parsed_name, header_linkname, 100 * sizeof(char));

    return parsed_name;
}


int is_x(int tar_fd, char *path, char *type_file)
{
    tar_header_t header;
    int ret = 0;

    lseek(tar_fd, 0, SEEK_SET);

    while (read(tar_fd, &header, HEADER_SIZE) > 0 && header.name[0] != '\0')
    {   
        if (strcmp(header.name, path) == 0)
        {
            if (strcmp(type_file, "dir") == 0)
            {
                if (header.typeflag == DIRTYPE)                                 {ret = 1; break;}
            }
            else if (strcmp(type_file, "file") == 0)
            {
                if (header.typeflag == REGTYPE || header.typeflag == AREGTYPE)  {ret = 1; break;}
            }
            else if (strcmp(type_file, "symlink") == 0)
            {
                if (header.typeflag == SYMTYPE || header.typeflag == LNKTYPE)   {ret = 1; break;}
            }
            else                                                                {ret = -1; break;}
        }
        skip_file_content(tar_fd, header);
    }

    lseek(tar_fd, 0, SEEK_SET);
    return ret;
}