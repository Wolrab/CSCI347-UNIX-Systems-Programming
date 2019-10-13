#ifndef __LS_H
#define __LS_H
#include <dirent.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include "dir_arr.h"

// All the options recognized by the program
#define OPT_STRING "a"

// The bitmask for each of the options
// LIMITED TO 7 OPTIONS WITH CHAR as -1 is reserved for an invalid option error
#define OPT_a_MASK 0x01

// Macros to add more information to perror and allocate the correct number of bytes
//   for the error string
#define OPENDIR_ERROR_F "opendir: cannot access '%s'"
#define OPENDIR_ERROR_NONF_LEN strlen(OPENDIR_ERROR_F)-2

// Actual ls functionality
int _ls(char *path, char options);

// Returns a maskable char with all the enabled options
char get_options(int argc, char **argv);

// Uses _add_entry to fill the tree list with all the entries of directory d
int get_dir_listings(DIR *d, qs *ent_names, char options);

// All data added to list must be added through _add_entry as there are no
//   guarantees about the long term storage of variables pointed to by dirent.
//   _add_entry manages the copying of those values.
int _add_entry(qs *ent_names, char *name);

#endif /* __LS_H */