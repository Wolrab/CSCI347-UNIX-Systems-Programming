#ifndef __LS_H
#define __LS_H
#include <dirent.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <sys/stat.h>
#include "f_list.h"

// All the options recognized by the program
#define OPT_STRING "al"

// The bitmask for each of the options
// LIMITED TO 7 OPTIONS WITH CHAR as -1 is reserved for an invalid option error
#define OPT_a_MASK 0x01
#define OPT_l_MASK 0x02

// Macros to add more information to perror and allocate the correct number of bytes
//   for the error string
#define OPENDIR_ERROR_F "opendir: cannot access '%s'"
#define OPENDIR_ERROR_NONF_LEN strlen(OPENDIR_ERROR_F)-2

// Actual ls functionality
int _ls(char *path, char options);

// Returns a maskable char with all the enabled options
char get_options(int argc, char **argv);

// Uses _add_entry to fill the f_list with the name of all the entries in d
int get_ent_names(DIR *d, f_list *dir_entries, const char options);

// Uses _add_entry to fill the f_list with all the names and stats of entries in d
int get_ent_stats(DIR *d, f_list *dir_entries, const char *path, const char options);

// All data added to list must be added through _add_entry as there are no
//   guarantees about the long term storage of d_name held in the dirent
//   structure. This manages the copying of that memory. The stat structure 
//   is fine because it's in our own buffer.
int _add_entry(f_list *dir_entries, char *ent_name, struct stat *ent_stat);

#endif /* __LS_H */