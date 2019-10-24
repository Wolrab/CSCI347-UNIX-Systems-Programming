#ifndef __LS_H
#define __LS_H
#include <dirent.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>
#include <assert.h>
#include <limits.h>
#include "list.h"
#include "long_out.h"

// All valid options for ls
#define OPTION_STRING "al"

// Option flags
// Show files begining with .
static bool option_a = false;
// Output files in long-format
static bool option_l = false;

// Error definitions
// Values are included for clarity
typedef enum ls_err ls_err;
enum ls_err {
    LS_ERR_NONE = 0,
    LS_ERR_MALLOC = 1,
    LS_ERR_DIR_STREAM_OPEN = 2,
    LS_ERR_DIR_STREAM_READ = 3,
    LS_ERR_STAT = 4,
    LS_ERR_DUP_ENTRY = 5,
    LS_ERR_PATH_OVERFLOW = 6,
    LS_ERR_LONG_PARSE = 7
};
extern const char *const ls_err_msg[];

// List all the directory entries of path. Outputs the result to standard out.
ls_err ls(char *path);

// Gets all the entries located in path and stores them in dir_entries.
ls_err get_entries(const char *path, list *dir_entries);

// Parses ent at path. The value is stored in dir_entries.
ls_err parse_entry(struct dirent *ent, const char *path, \
        list *dir_entries);

// Gets a file's full path from f_name and the path of the directory that 
//   contains it, storing it in path_buf.
ls_err get_full_path(char *path_buf, int path_buf_len, const char *f_name, \
    const char *path);

// Outputs all entries of dir_entries to stdout.
void output_entries(list *dir_entries);

// Output all entries of dir_entries to stdout using the long-format output.
ls_err output_entries_long(list *dir_entries);

// Sets the option flags given an array of arguments and their size.
int get_options(const int argc, char **argv);

// Basic error output for ls.
void ls_perror(ls_err err, char *pname);

#endif /* __LS_H */