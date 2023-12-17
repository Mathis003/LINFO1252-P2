#include "lib_tar.h"
#include <stdbool.h>

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

    lseek(tar_fd, 0, SEEK_SET);

    while (1)
    { 
        if (read(tar_fd, &header, HEADER_SIZE) <= 0 || header.name[0] == '\0') break;

        // get_info_header(header, nber_valid_headers); // Help to Debug

        // Vérifie la valeur "magic" et "version"
        if (strncmp(header.magic, TMAGIC, TMAGLEN) != 0)      return -1;
        if (strncmp(header.version, TVERSION, TVERSLEN) != 0) return -2;

        // Calcule le checksum
        long int header_chksum = TAR_INT(header.chksum);
        memset(header.chksum, ' ', 8);

        long int chksum_calculated = 0;
        uint8_t *current_byte = (uint8_t *) &header;

        for (int i = 0; i < HEADER_SIZE; i++) chksum_calculated += *(current_byte + i);

        // Vérifie le checksum
        if (header_chksum != chksum_calculated) return -3;

        if (header.typeflag == REGTYPE || header.typeflag == AREGTYPE) lseek(tar_fd, HEADER_SIZE * (1 + TAR_INT(header.size) / HEADER_SIZE), SEEK_CUR);
        
        nber_valid_headers++;
    }

    return nber_valid_headers;
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

    lseek(tar_fd, 0, SEEK_SET);

    while (1)
    {
        if (read(tar_fd, &header, HEADER_SIZE) <= 0 || header.name[0] == '\0') break;
        if (strcmp(header.name, path) == 0) return 1;
        if (header.typeflag == REGTYPE || header.typeflag == AREGTYPE) lseek(tar_fd, HEADER_SIZE * (1 + TAR_INT(header.size) / HEADER_SIZE), SEEK_CUR);
    }

    return 0;
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

    lseek(tar_fd, 0, SEEK_SET);

    while (1)
    {
        if (read(tar_fd, &header, HEADER_SIZE) <= 0 || header.name[0] == '\0') break;
        
        if (strcmp(header.name, path) == 0)
        {
            if (strcmp(type_file, "dir") == 0)
            {
                if (header.typeflag == DIRTYPE)                                 return 1;
            }
            else if (strcmp(type_file, "file") == 0)
            {
                if (header.typeflag == REGTYPE || header.typeflag == AREGTYPE)  return 1;
            }
            else if (strcmp(type_file, "symlink") == 0)
            {
                if (header.typeflag == SYMTYPE || header.typeflag == LNKTYPE)   return 1;
            }
            else                                                                return -1;
        }

        if (header.typeflag == REGTYPE || header.typeflag == AREGTYPE) lseek(tar_fd, HEADER_SIZE * (1 + TAR_INT(header.size) / HEADER_SIZE), SEEK_CUR);
    }
    
    return 0;
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
 * Skips the directory entries in a tar archive until a different directory is encountered.
 *
 * This function reads the tar archive file descriptor 'tar_fd' and advances the
 * 'header' to the next entry, skipping entries with the same directory name as the
 * current 'header->name'. It continues until a different directory entry is found.
 *
 * @param tar_fd The file descriptor of the tar archive.
 * @param header A pointer to the current tar header structure.
 */
void skip_dir(int tar_fd, tar_header_t *header, int *count)
{
    char *name_dir = strdup(header->name);
    while (strncmp(name_dir, header->name, strlen(name_dir)) == 0)
    {
        // printf("SKIPPING SUBDIR\theader_name %d : %s\n", *count, header->name);
        // (*count)++;

        if (header->typeflag == REGTYPE || header->typeflag == AREGTYPE) lseek(tar_fd, HEADER_SIZE * (1 + TAR_INT(header->size) / HEADER_SIZE), SEEK_CUR);
        if (read(tar_fd, header, HEADER_SIZE) <= 0 || header->name[0] == '\0') break;
    }
    free(name_dir);
    // printf("ENTRY\t\theader_name %d : %s\n", *count, header->name);
}

/**
 * Adds a new entry to the list of entries.
 *
 * This function appends the specified entry, represented by the 'name_entry' parameter,
 * to the list of entries. The current count of listed entries is maintained in the
 * 'listed_entries' parameter.
 *
 * @param entries A pointer to the list of entries.
 * @param name_entry The name of the entry to be added.
 * @param listed_entries A pointer to the count of listed entries.
 * @param nber_entries The maximum number of entries that the list can accommodate.
 *
 * @return -1 if the list is full and cannot accommodate more entries,
 *          0 otherwise (entry added successfully).
 */
int list_new_entry(char **entries, char *name_entry, size_t *listed_entries, int nber_entries)
{
    if (nber_entries <= *listed_entries) return -1;
    memcpy(entries[*listed_entries], name_entry, strlen(name_entry));
    (*listed_entries)++;
    return 0;
}

void parse_symlink(tar_header_t header, char *final_name)
{
    char *lastSlash = strrchr(header.name, '/');

    if (lastSlash != NULL)
    {   
        strncpy(final_name, header.name, lastSlash - header.name + 1);
        final_name[lastSlash - header.name + 1] = '\0';
        strncat(final_name, header.linkname, strlen(header.linkname));
    }
    else
    {
        strncpy(final_name, header.linkname, strlen(header.linkname));
        final_name[strlen(header.linkname)] = '\0';
    }
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
    int count = 0; // To Debug only

    lseek(tar_fd, 0, SEEK_SET);

    while (read(tar_fd, &header, HEADER_SIZE) > 0 && header.name[0] != '\0')
    {
        if (header.typeflag == REGTYPE || header.typeflag == AREGTYPE) lseek(tar_fd, HEADER_SIZE * (1 + TAR_INT(header.size) / HEADER_SIZE), SEEK_CUR);

        // printf("SEARCHING DIR\theader_name %d : %s\n", count, header.name);
        // count++;
        if (strcmp(header.name, path) != 0) continue;

        if (header.typeflag == REGTYPE || header.typeflag == AREGTYPE) break;
        else if (header.typeflag == SYMTYPE || header.typeflag == LNKTYPE)
        {
            char *final_name = (char *) malloc(500 * sizeof(char));
            parse_symlink(header, final_name);
            if (is_symlink(tar_fd, final_name) == 0) strcat(final_name, "/");
            int result = list(tar_fd, final_name, entries, no_entries);
            free(final_name);
            return result;
        }
        else if (header.typeflag == DIRTYPE)
        {
            // printf("DIR FOUNDED\tname : %s\n", header.name);
            char *name_dir = strdup(header.name);
            dir_founded = 1;

            if (read(tar_fd, &header, HEADER_SIZE) <= 0 || header.name[0] == '\0') {free(name_dir); break;}

            while (strncmp(header.name, name_dir, strlen(name_dir)) == 0)
            {
                // printf("ENTRY\t\theader name %d : %s\n", count, header.name);
                // count++;
                if (list_new_entry(entries, header.name, &listed_entries, *no_entries) == -1) break;
                
                if (header.typeflag == DIRTYPE) skip_dir(tar_fd, &header, &count);
                else
                {
                    if (header.typeflag == REGTYPE || header.typeflag == AREGTYPE) lseek(tar_fd, HEADER_SIZE * (1 + TAR_INT(header.size) / HEADER_SIZE), SEEK_CUR);
                    if (read(tar_fd, &header, HEADER_SIZE) <= 0 || header.name[0] == '\0') break;
                }
            }
            free(name_dir);
            break;
        }
    }

    *no_entries = listed_entries;
    if (dir_founded == 1 && (*no_entries) == 0) return 1;
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
    if ((int) offset < 0) { *len = 0; return -2; }

    lseek(tar_fd, 0, SEEK_SET);

    while (1)
    {
        if (read(tar_fd, &header, HEADER_SIZE) <= 0 || header.name[0] == '\0') break;

        if (strcmp(header.name, path) == 0)
        {
            if (header.typeflag == SYMTYPE || header.typeflag == LNKTYPE) return read_file(tar_fd, header.linkname, offset, dest, len);
            if (header.typeflag == AREGTYPE || header.typeflag == REGTYPE)
            {
                long total_len = TAR_INT(header.size) - offset;
                if (total_len <= 0) { *len = 0; return -2; }

                lseek(tar_fd, offset, SEEK_CUR);

                long used_len = (total_len > dest_len) ? dest_len : total_len;
                if (read(tar_fd, dest, used_len) <= 0) break;

                *len = used_len;
                return total_len - used_len;
            }
        }
        
        if (header.typeflag == REGTYPE || header.typeflag == AREGTYPE) lseek(tar_fd, HEADER_SIZE * (1 + TAR_INT(header.size) / HEADER_SIZE), SEEK_CUR);
    }

    *len = 0;
    return -1;
}