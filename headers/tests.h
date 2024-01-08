#ifndef TESTS_H
#define TESTS_H

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

#include "lib_tar.h"

/**
 * @brief Display a hexadecimal dump of a byte array.
 *
 * @param bytes The byte array to be dumped.
 * @param len   The length of the byte array.
 */
void debug_dump(const uint8_t *bytes, size_t len);

/**
 * @brief Test function for the check_archive function.
 *
 * @param fd       File descriptor of the tar archive.
 * @param expected Expected return value.
 */
void check_archive_test(int fd, int expected);

/**
 * @brief Test function for the exists function.
 *
 * @param fd       File descriptor of the tar archive.
 * @param path     Path to check for existence.
 * @param expected Expected return value.
 */
void exists_test(int fd, char *path, int expected);

/**
 * @brief Test function for various file type checking functions (is_file, is_dir, is_symlink).
 *
 * @param fd       File descriptor of the tar archive.
 * @param path     Path to check.
 * @param type     Type of file to check ("file", "dir", "symlink").
 * @param expected Expected return value.
 */
void is_x_test(int fd, char *path, char *type, int expected);

/**
 * @brief Comparison function for sorting strings.
 *
 * @param a Pointer to the first string.
 * @param b Pointer to the second string.
 * @return   Result of the string comparison.
 */
int cmper_str(const void *a, const void *b);

/**
 * @brief Test function for the list function.
 *
 * @param fd                  File descriptor of the tar archive.
 * @param path                Path to list.
 * @param no_entries          Maximum number of entries to list.
 * @param expected_ret        Expected return value.
 * @param expected_no_entries Expected number of entries.
 * @param expected_entries    Expected array of entry names.
 */
void list_test(int fd, char *path, size_t no_entries, int expected_ret, size_t expected_no_entries, char *expected_entries[]);

/**
 * @brief Test function for the read_file function.
 *
 * @param fd              File descriptor of the tar archive.
 * @param path            Path to the file to read.
 * @param offset          Offset from the beginning of the file.
 * @param len             Maximum length to read.
 * @param expected_ret    Expected return value.
 * @param expected_len    Expected length of the read data.
 * @param expected_buffer Expected content of the read data.
 */
void read_file_test(int fd, char *path, size_t offset, size_t len, int expected_ret, size_t expected_len, char *expected_buffer);

/**
 * @brief Main test function.
 *
 * @param argc Number of command-line arguments.
 * @param argv Array of command-line arguments.
 * @return     Exit status.
 */
int main(int argc, char **argv);

#endif /* TESTS_H */