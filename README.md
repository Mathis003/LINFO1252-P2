# Tar Archive Utility in C

## Description

Welcome to the project repository for the LINFO1252 course at UCLouvain, part of the Civil Engineering program in the field of computer science. This project focuses on implementing a basic Tar archive utility in C, providing functionality for creating, reading, and manipulating Tar archives. The utility includes operations such as checking the validity of an archive, checking for the existence of a file or directory, determining the type of a file (regular file, directory, or symbolic link), listing the contents of a directory within the archive, and reading the contents of a file.

### Project Details
- **Grade:** 20/20.
- **Course:** LINFO1252, UCLouvain Engineering program, Computer Science track.

## Tar Archive Structure

The Tar archive follows the Tar format standards, with each file entry represented by a Tar header. The utility supports essential Tar header fields such as name, size, type flag, magic, version, and checksum. The project emphasizes multithreaded performance analysis using three classical concurrent programming problems: the Reader-Writer problem, the Dining Philosophers problem, and the Producer-Consumer problem.

## Features

### 1. Checking Archive Validity

The function `check_archive` verifies the validity of a Tar archive by inspecting its headers. It ensures that the magic and version fields match the Tar standards and validates the checksum for each header.

### 2. File and Directory Existence Check

The `exists` function checks if a file or directory exists in the Tar archive. It iterates through the archive's headers and compares the provided path with each entry.

### 3. Type Checking (File, Directory, Symlink)

The functions `is_file`, `is_dir`, and `is_symlink` determine the type of a specified path within the Tar archive. They leverage type flags in Tar headers to identify regular files, directories, and symbolic links.

### 4. Listing Directory Contents

The `list` function lists the contents of a specified directory within the Tar archive. It supports recursive listing, providing entries and the number of entries found.

### 5. Reading File Contents

The `read_file` function reads the contents of a specified file within the Tar archive. It supports specifying an offset for partial reads and provides the read data and remaining length.

## Makefile Commands

This project uses a Makefile to streamline compilation, execution, and additional tasks. Here are the main commands:

- **`make`**: Uses `make build` and `make run`.
- **`make build`**: Compiles the project, generating the executable `my_program` and the Tar archive `TAR_archive_test.tar`.
- **`make run`**: Executes the compiled program (`my_program`).
- **`make clean`**: Removes generated files and the executable.
- **`make tar`**: Creates a Tar archive (`TAR_archive_test.tar`) containing all the files in the directory `archive_test`.
- **`make submit`**: Creates a submission Tar archive (`soumission.tar`) containing source files, headers, and the Makefile.

## Further Information

For additional details on function parameters, return values, and specific implementation details, refer to the source code and header files.
Please note that this version represents a partial implementation that may lack certain features. Notably, the functionality for listing directory contents (`list` function) does not currently support relative paths (e.g., the use of `../`). This limitation may impact the accurate representation of directory structures in certain cases.

Contributors are aware of this limitation, and future updates or versions may address these aspects for a more comprehensive solution.
For detailed information on the current state of the project, refer to the source code and associated documentation.

## Contributions

Contributions, feedback, and suggestions are welcomed. Feel free to share your thoughts and improvements.

---
*This project is a part of the LINFO1252 course at UCL in the Civil Engineering program, focusing on computer science.*