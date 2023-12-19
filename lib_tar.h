#ifndef LIB_TAR_H
#define LIB_TAR_H

#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

typedef struct posix_header
{                              /* byte offset */
    char name[100];               /*   0 */
    char mode[8];                 /* 100 */
    char uid[8];                  /* 108 */
    char gid[8];                  /* 116 */
    char size[12];                /* 124 */
    char mtime[12];               /* 136 */
    char chksum[8];               /* 148 */
    char typeflag;                /* 156 */
    char linkname[100];           /* 157 */
    char magic[6];                /* 257 */
    char version[2];              /* 263 */
    char uname[32];               /* 265 */
    char gname[32];               /* 297 */
    char devmajor[8];             /* 329 */
    char devminor[8];             /* 337 */
    char prefix[155];             /* 345 */
    char padding[12];             /* 500 */
} tar_header_t;

#define HEADER_SIZE sizeof(tar_header_t)

#define TMAGIC   "ustar"        /* ustar and a null */
#define TMAGLEN  6
#define TVERSION "00"           /* 00 and no null */
#define TVERSLEN 2

/* Values used in typeflag field.  */
#define REGTYPE  '0'            /* regular file */
#define AREGTYPE '\0'           /* regular file */
#define LNKTYPE  '1'            /* link */
#define SYMTYPE  '2'            /* reserved */
#define DIRTYPE  '5'            /* directory */

/* Converts an ASCII-encoded octal-based number into a regular integer */
#define TAR_INT(char_ptr) strtol(char_ptr, NULL, 8)


/*
Helpful functions : BEGIN
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
void get_info_header(tar_header_t header, int id);

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
void skip_file_content(int tar_fd, tar_header_t header);

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
void skip_dir(int tar_fd, tar_header_t *header);

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
int check_if_entry_folder(char *parent_dir, char *current_path);

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
char *parse_symlink(char *header_name, char *header_linkname);

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
int is_x(int tar_fd, char *path, char *type_file);

/*
Helpful functions : END
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
int check_archive(int tar_fd);

/**
 * Checks whether an entry exists in the archive.
 *
 * @param tar_fd A file descriptor pointing to the start of a valid tar archive file.
 * @param path A path to an entry in the archive.
 *
 * @return zero if no entry at the given path exists in the archive,
 *         any other value otherwise.
 */
int exists(int tar_fd, char *path);

/**
 * Checks whether an entry exists in the archive and is a directory.
 *
 * @param tar_fd A file descriptor pointing to the start of a valid tar archive file.
 * @param path A path to an entry in the archive.
 *
 * @return zero if no entry at the given path exists in the archive or the entry is not a directory,
 *         any other value otherwise.
 */
int is_dir(int tar_fd, char *path);

/**
 * Checks whether an entry exists in the archive and is a file.
 *
 * @param tar_fd A file descriptor pointing to the start of a valid tar archive file.
 * @param path A path to an entry in the archive.
 *
 * @return zero if no entry at the given path exists in the archive or the entry is not a file,
 *         any other value otherwise.
 */
int is_file(int tar_fd, char *path);

/**
 * Checks whether an entry exists in the archive and is a symlink.
 *
 * @param tar_fd A file descriptor pointing to the start of a valid tar archive file.
 * @param path A path to an entry in the archive.
 *
 * @return zero if no entry at the given path exists in the archive or the entry is not symlink,
 *         any other value otherwise.
 */
int is_symlink(int tar_fd, char *path);

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
int list(int tar_fd, char *path, char **entries, size_t *no_entries);

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
ssize_t read_file(int tar_fd, char *path, size_t offset, uint8_t *dest, size_t *len);

#endif //LIB_TAR_H