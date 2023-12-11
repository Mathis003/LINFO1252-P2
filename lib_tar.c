#include "lib_tar.h"

#include <stdio.h>

const size_t HEADER_SIZE = sizeof(tar_header_t);

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
    int valid_headers = 0;

    while (1)
    {
        ssize_t bytes_read = read(tar_fd, &header, HEADER_SIZE);
        if (bytes_read == 0 || bytes_read != HEADER_SIZE) break;

        if (header.name[0] != '\0') break;
        
        // Check magic value
        if (strncmp(header.magic, TMAGIC, TMAGLEN) != 0)
        {
            close(tar_fd);
            return -1;
        }

        // Check version value
        if (strncmp(header.version, TVERSION, TVERSLEN) != 0)
        {
            close(tar_fd);
            return -2;
        }

        // Check checksum value
        if (header.chksum[0] == '\0')
        {
            close(tar_fd);
            return -3;
        }

        valid_headers++;
    }

    close(tar_fd);
    return valid_headers;
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
    while (1)
    {
        ssize_t bytes_read = read(tar_fd, &header, HEADER_SIZE);
        if (bytes_read == 0 || bytes_read != HEADER_SIZE) break;

        if (header.name[0] == '\0') break;

        // Check if the entry exists
        if (strcmp(header.name, path) == 0) return 1;
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
 *         0 otherwise. If the specified type is not recognized, the function returns 0.
 */
int is_x(int tar_fd, char *path, char *types_file)
{
    tar_header_t header;
    while (1)
    {
        ssize_t bytes_read = read(tar_fd, &header, HEADER_SIZE);
        if (bytes_read == 0 || bytes_read != HEADER_SIZE) break;

        if (header.name[0] == '\0') break;
        
        // Check if the entry exists
        if (strcmp(header.name, path) == 0)
        {
            // Check if the entry matches the specified type
            if (((strlen(types_file) == 1) && (header.typeflag == types_file[0])) ||
               ((strlen(types_file) == 2) && (header.typeflag == types_file[0] || header.typeflag == types_file[1])))
                return 1;
        }
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
    char type_file[1];
    type_file[0] = DIRTYPE;
    return is_x(tar_fd, path, type_file);
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
    char types_file[2];
    types_file[0] = REGTYPE;
    types_file[1] = AREGTYPE;
    return is_x(tar_fd, path, types_file);
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
    char types_file[2];
    types_file[0] = SYMTYPE;
    types_file[1] = LNKTYPE;
    return is_x(tar_fd, path, types_file);
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
    int iter = 0;
    *no_entries = 0;
    while (1)
    {
        ssize_t bytes_read = read(tar_fd, &header, HEADER_SIZE);
        if (bytes_read == 0 || bytes_read != HEADER_SIZE) break;

        if (header.name[0] == '\0') break;
        
        // Check if the entry exists
        if (strcmp(header.name, path) == 0)
        {
            if (header.typeflag == SYMTYPE || header.typeflag == LNKTYPE) list(tar_fd, header.linkname, entries, no_entries);
            else
            {
                entries[iter] = header.name;
                (*no_entries)++;
                iter++;
            }
        }
    }
    return *no_entries;
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
    size_t max_length = *len;
    if (offset < 0 || offset > max_length) return -2;

    tar_header_t header;
    while (1)
    {
        ssize_t bytes_read = pread(tar_fd, &header, HEADER_SIZE, offset);
        if (bytes_read == 0 || bytes_read != HEADER_SIZE) break;

        if (header.name[0] == '\0') break;
        
        if (strcmp(header.name, path) == 0)
        {
            if (header.typeflag == SYMTYPE || header.typeflag == LNKTYPE) read_file(tar_fd, header.linkname, offset, dest, len);

            bytes_read = pread(tar_fd, dest, TAR_INT(header.size), offset);
            *len += bytes_read;
            offset += TAR_INT(header.size);
        }
    }

    return (*len == max_length) ? 0 : (*len - max_length);
}                