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

typedef enum ls_err ls_err;
enum ls_err {
    LS_NONE,
    LS_MALLOC_ERR, // errno: meaningful
    LS_DIR_ACC_ERR, // errno: meaningful
    LS_DIR_READ_ENTRY_ERR, // errno: meaningful
    LS_LSTAT_ERR // errno: meaningful
} ls_err_state = LS_NONE;

static const char *ls_err_out[] = {
    "no error",
    "malloc",
    "opendir",
    "readdir",
    "lstat"
};

void ls_perror(char *prog);

// Actual ls functionality
int _ls(char *path, char options);

// Returns a maskable char with all the enabled options
char get_options(int argc, char **argv);

// Uses _add_entry to fill the f_list with the name of all the entries in d
int get_ent_names(DIR *d, f_list *dir_entries, const char options);

// Uses _add_entry to fill the f_list with all the names and stats of entries in d
int get_ent_stats(DIR *d, f_list *dir_entries, const char *path, const char options);

#endif /* __LS_H */