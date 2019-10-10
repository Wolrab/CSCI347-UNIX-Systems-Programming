#ifndef __LS_H
#define __LS_H
#include <dirent.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include "bst.h"

// All the options recognized by the program
#define OPT_STRING "a"

// The bitmask for each of the options
#define OPT_a_MASK 0x01

// Macros to add more information to perror and allocate the correct number of bytes
#define OPENDIR_ERROR_F "opendir: can't open '%s'"
#define OPENDIR_ERROR_NONF_LEN strlen(OPENDIR_ERROR_F)-2

// Returns a maskable char with all the enabled options
char get_options(int argc, char **argv);

// Uses _add_entry to fill the tree list with all the entries of directory d
int get_dir_listings(DIR *d, tree *list, char options);

// All data added to list must be added through _add_entry as there are no
//   guarantees about the long term storage of variables pointed to by dirent
//   and _add_entry manages copying of those values 
int _add_entry(tree *list, char *name);

#endif /* __LS_H */