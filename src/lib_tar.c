#include "../headers/lib_tar.h"

int check_archive(int tar_fd)
{
    tar_header_t header;
    int nber_valid_headers = 0;
    int ret = 0;

    lseek(tar_fd, 0, SEEK_SET);

    while (read(tar_fd, &header, HEADER_SIZE) > 0 && header.name[0] != '\0')
    { 
        // Vérifie la valeur "magic" et "version"
        if (strncmp(header.magic, TMAGIC, TMAGLEN) != 0)      {ret = -1; break;}
        if (strncmp(header.version, TVERSION, TVERSLEN) != 0) {ret = -2; break;}

        // Calcule le checksum
        long int header_chksum = TAR_INT(header.chksum);
        memset(header.chksum, ' ', 8);

        long int chksum_calculated = 0;
        uint8_t *current_byte = (uint8_t *) &header;

        for (int i = 0; i < HEADER_SIZE; i++) chksum_calculated += *(current_byte + i);

        // Vérifie le checksum
        if (header_chksum != chksum_calculated) {ret = -3; break;}

        skip_file_content(tar_fd, header);
        nber_valid_headers++;
    }

    lseek(tar_fd, 0, SEEK_SET);
    return (ret == 0) ? nber_valid_headers : ret;
}


int exists(int tar_fd, char *path)
{
    tar_header_t header;
    int ret = 0;

    lseek(tar_fd, 0, SEEK_SET);

    while (read(tar_fd, &header, HEADER_SIZE) > 0 && header.name[0] != '\0')
    {
        if (strcmp(header.name, path) == 0) {ret = 1; break;}
        skip_file_content(tar_fd, header);
    }

    lseek(tar_fd, 0, SEEK_SET);
    return ret;
}


int is_dir(int tar_fd, char *path) { return is_x(tar_fd, path, "dir"); }


int is_file(int tar_fd, char *path) { return is_x(tar_fd, path, "file"); }


int is_symlink(int tar_fd, char *path) { return is_x(tar_fd, path, "symlink"); }


int list(int tar_fd, char *path, char **entries, size_t *no_entries)
{
    tar_header_t header;
    size_t listed_entries = 0;
    int dir_founded = 0;

    lseek(tar_fd, 0, SEEK_SET);

    while (read(tar_fd, &header, HEADER_SIZE) > 0 && header.name[0] != '\0')
    {
        skip_file_content(tar_fd, header);

        // Continue the loop until we find the path
        if (strcmp(header.name, path) != 0) continue;


        if (header.typeflag == REGTYPE || header.typeflag == AREGTYPE) break;
        else if (header.typeflag == SYMTYPE || header.typeflag == LNKTYPE)
        {
            char *parsed_name = parse_symlink(header.name, header.linkname);
            if (is_symlink(tar_fd, parsed_name) == 0) strcat(parsed_name, "/");
            int result =  list(tar_fd, parsed_name, entries, no_entries);
            free(parsed_name);
            return result;
        }
        else if (header.typeflag == DIRTYPE)
        {
            dir_founded = 1;
            char *name_dir = (char *) malloc(100 * sizeof(char));
            memcpy(name_dir, header.name, 100 * sizeof(char));

            if (read(tar_fd, &header, HEADER_SIZE) <= 0 || header.name[0] == '\0') {free(name_dir); break;}

            while (check_if_entry_folder(name_dir, header.name) == 1)
            {
                if (*no_entries <= listed_entries) break;
                memcpy(entries[listed_entries], header.name, 100 * sizeof(char));
                listed_entries++;

                // Skip the subdirectory
                if (header.typeflag == DIRTYPE) skip_dir(tar_fd, &header);
                else
                {
                    skip_file_content(tar_fd, header);
                    if (read(tar_fd, &header, HEADER_SIZE) <= 0 || header.name[0] == '\0') break;
                }
            }

            free(name_dir);
            break;
        }
    }

    *no_entries = listed_entries;
    if (dir_founded == 1 && listed_entries == 0) return 1;
    return (listed_entries > 0) ? 1 : 0;
}


ssize_t read_file(int tar_fd, char *path, size_t offset, uint8_t *dest, size_t *len)
{
    tar_header_t header;
    size_t dest_len = *len;
    if ((int) offset < 0) {*len = 0; return -2;}
    int ret = -1;

    lseek(tar_fd, 0, SEEK_SET);

    while (read(tar_fd, &header, HEADER_SIZE) > 0 && header.name[0] != '\0')
    {
        if (strcmp(header.name, path) == 0)
        {
            if (header.typeflag == SYMTYPE || header.typeflag == LNKTYPE) return read_file(tar_fd, header.linkname, offset, dest, len);
            if (header.typeflag == AREGTYPE || header.typeflag == REGTYPE)
            {
                size_t total_len = TAR_INT(header.size) - offset;
                if (total_len <= 0) {ret = -2; break;}

                lseek(tar_fd, offset, SEEK_CUR);

                size_t used_len = (total_len > dest_len) ? dest_len : total_len;
                if (read(tar_fd, dest, used_len) <= 0) break;
                *len = used_len;
                ret = total_len - used_len;
                break;
            }
        }
        skip_file_content(tar_fd, header);
    }
    
    if (ret < 0) *len = 0;
    lseek(tar_fd, 0, SEEK_SET);
    return ret;
}