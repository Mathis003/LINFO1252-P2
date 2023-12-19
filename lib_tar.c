#include "lib_tar.h"

/*
Helper functions : BEGIN
*/

/**
 * Prints information about a tar header for debugging or informational purposes.
 *
 * This function outputs details of the specified 'header' with the provided 'id'
 * to the console, including the entry's name, size, typeflag, magic, version, and checksum.
 *
 * @param header The tar header structure to display information for.
 * @param id The identifier or sequence number for the header (for display purposes).
 */
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

/**
 * Skips the content of a file within a tar archive based on the provided tar header information.
 *
 * This function advances the file pointer within the tar archive file descriptor ('tar_fd') to
 * skip the content of the file associated with the given 'header'. The skipping is performed
 * based on the file size specified in the tar header, ensuring that the file content is
 * effectively skipped, and the file pointer is positioned at the next tar header or the end of
 * the tar archive.
 *
 * @param tar_fd The file descriptor of the tar archive.
 * @param header The tar header structure containing information about the file to skip.
 */
void skip_file_content(int tar_fd, tar_header_t header)
{
    int nb_blocks = TAR_INT(header.size) / HEADER_SIZE;
    if (TAR_INT(header.size) % HEADER_SIZE != 0) nb_blocks++;
    lseek(tar_fd, HEADER_SIZE * nb_blocks, SEEK_CUR);
}

/**
 * Skips the directory entries in a tar archive until a different directory is encountered.
 *
 * This function reads the tar archive file descriptor 'tar_fd' and advances the
 * 'header' to the next entry, skipping entries with the same directory name as the
 * current 'header->name'. It continues until a different directory entry is found.
 *
 * @param tar_fd The file descriptor of the tar archive.
 * @param header A pointer to the current tar header structure.
 */
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

/**
 * Checks if the current path belongs to the same directory as the parent directory.
 *
 * This function compares the 'parent_dir' and 'current_path' character strings to determine
 * whether they represent entries in the same directory. It returns 1 if 'current_path' is
 * a subdirectory or a file within 'parent_dir', and 0 otherwise.
 *
 * @param parent_dir A null-terminated character string representing the parent directory.
 * @param current_path A null-terminated character string representing the current directory path.
 * @return Returns 1 if 'current_path' is within or equal to 'parent_dir', and 0 otherwise.
 */
int check_if_entry_folder(char *parent_dir, char *current_path)
{
    for (int i = 0; parent_dir[i] != '\0' ; i++)
    {
        if (current_path[i] == '\0' || parent_dir[i] != current_path[i]) return 0;
    }
    return 1;
}

/**
 * Parses the symlink path based on the header information.
 *
 * This function constructs the parsed symlink path by combining the base directory
 * from the 'header_name' and the relative symlink path from 'header_linkname'. The
 * resulting parsed symlink path is returned as a dynamically allocated string.
 *
 * @param header_name A null-terminated character string representing the base directory.
 * @param header_linkname A null-terminated character string representing the symlink path.
 * @return Returns the parsed symlink path as a dynamically allocated string.
 *         If the allocation fails or the resulting path exceeds 100 characters, an
 *         empty string is returned.
 */
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

/**
 * Checks whether an entry in the archive matches the specified type.
 *
 * @param tar_fd A file descriptor pointing to the start of a valid tar archive file.
 * @param path A path to an entry in the archive.
 * @param type_file String(s) specifying the expected type(s) of the entry ([DIRTYPE], [REGTYPE, AREGTYPE], [SYMTYPE, LNKTYPE]).
 *
 * @return 1 if the entry at the given path exists in the archive and matches the specified type,
 *         0 otherwise. If the specified type is not recognized, the function returns -1.
 */
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

/*
Helper functions : END
*/


/**
 * Checks whether the archive is valid.
 *
 * Each non-null header of a valid archive has:
 *  - a magic value of "ustar" and a null,
 *  - a version value of "00" and no null,
 *  - a correct checksum
 *
 * @param tar_fd A file descriptor pointing to the start of a file supposed to contain a tar archive.
 *
 * @return a zero or positive value if the archive is valid, representing the number of non-null headers in the archive,
 *         -1 if the archive contains a header with an invalid magic value,
 *         -2 if the archive contains a header with an invalid version value,
 *         -3 if the archive contains a header with an invalid checksum value
 */
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

/**
 * Checks whether an entry exists in the archive.
 *
 * @param tar_fd A file descriptor pointing to the start of a valid tar archive file.
 * @param path A path to an entry in the archive.
 *
 * @return zero if no entry at the given path exists in the archive,
 *         any other value otherwise.
 */
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


/**
 * Checks whether an entry exists in the archive and is a directory.
 *
 * @param tar_fd A file descriptor pointing to the start of a valid tar archive file.
 * @param path A path to an entry in the archive.
 *
 * @return zero if no entry at the given path exists in the archive or the entry is not a directory,
 *         any other value otherwise.
 */
int is_dir(int tar_fd, char *path)
{
    return is_x(tar_fd, path, "dir");
}

/**
 * Checks whether an entry exists in the archive and is a file.
 *
 * @param tar_fd A file descriptor pointing to the start of a valid tar archive file.
 * @param path A path to an entry in the archive.
 *
 * @return zero if no entry at the given path exists in the archive or the entry is not a file,
 *         any other value otherwise.
 */
int is_file(int tar_fd, char *path)
{
    return is_x(tar_fd, path, "file");
}

/**
 * Checks whether an entry exists in the archive and is a symlink.
 *
 * @param tar_fd A file descriptor pointing to the start of a valid tar archive file.
 * @param path A path to an entry in the archive.
 * @return zero if no entry at the given path exists in the archive or the entry is not symlink,
 *         any other value otherwise.
 */
int is_symlink(int tar_fd, char *path)
{
    return is_x(tar_fd, path, "symlink");
}

/**
 * Lists the entries at a given path in the archive.
 * list() does not recurse into the directories listed at the given path.
 *
 * Example:
 *  dir/          list(..., "dir/", ...) lists "dir/a", "dir/b", "dir/c/" and "dir/e/"
 *   ├── a
 *   ├── b
 *   ├── c/
 *   │   └── d
 *   └── e/
 *
 * @param tar_fd A file descriptor pointing to the start of a valid tar archive file.
 * @param path A path to an entry in the archive. If the entry is a symlink, it must be resolved to its linked-to entry.
 * @param entries An array of char arrays, each one is long enough to contain a tar entry path.
 * @param no_entries An in-out argument.
 *                   The caller set it to the number of entries in `entries`.
 *                   The callee set it to the number of entries listed.
 *
 * @return zero if no directory at the given path exists in the archive,
 *         any other value otherwise.
 */
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


/**
 * Reads a file at a given path in the archive.
 *
 * @param tar_fd A file descriptor pointing to the start of a valid tar archive file.
 * @param path A path to an entry in the archive to read from.  If the entry is a symlink, it must be resolved to its linked-to entry.
 * @param offset An offset in the file from which to start reading from, zero indicates the start of the file.
 * @param dest A destination buffer to read the given file into.
 * @param len An in-out argument.
 *            The caller set it to the size of dest.
 *            The callee set it to the number of bytes written to dest.
 *
 * @return -1 if no entry at the given path exists in the archive or the entry is not a file,
 *         -2 if the offset is outside the file total length,
 *         zero if the file was read in its entirety into the destination buffer,
 *         a positive value if the file was partially read, representing the remaining bytes left to be read to reach
 *         the end of the file.
 *
 */
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
                long total_len = TAR_INT(header.size) - offset;
                if (total_len <= 0) {ret = -2; break;}

                lseek(tar_fd, offset, SEEK_CUR);

                long used_len = (total_len > dest_len) ? dest_len : total_len;
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