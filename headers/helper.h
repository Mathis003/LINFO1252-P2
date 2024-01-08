#ifndef HELPER_H
#define HELPER_H

#include "var.h"

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

#endif /* HELPER_H */