#ifndef __LONG_OUT_H
#define __LONG_OUT_H
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <stdbool.h>
#include "print_utils.h"
#include "list.h"

// Defines for long_out_s
#define MODE_STR_LEN 11
#define DATE_STR_LEN 13

// Error defines
typedef enum l_out_err l_out_err;
enum l_out_err {
    L_OUT_ERR_NONE   = 0,
    L_OUT_ERR_MALLOC = 1,
    L_OUT_ERR_PARSE  = 2
};

// Stores relevant values for long-format output
struct long_out_s {
    char *ino_str;
    char mode_str[MODE_STR_LEN];
    char *nlink_str;
    char *usr_str;
    char *grp_str;
    char *size_str;
    char mtim_str[DATE_STR_LEN];
    char *f_name;
};

struct dir_long_out_s {
    int ino_str_max;
    int nlink_str_max;
    int usr_str_max;
    int grp_str_max;
    int size_str_max;

    struct long_out_s *entries;
    int entries_c;
};

// Initializes dir_long_out.
void dir_long_out_init(struct dir_long_out_s *dir_long_out);

// Creates a dir_long_out with a long_out for every entry and formatting
//   information. dir_long_out must be initialized.
l_out_err dir_long_out_create(struct dir_long_out_s *dir_long_out, 
        list *dir_entries);

// Helper function for dir_long_out_create
// Sets all the maximum string values for dir_long_out.
void set_max_strs(struct dir_long_out_s *dir_long_out);

// Fills the entries of long_out with data for long-format output.
//   long_out must already be allocated. f_name is not copied and the user
//   must handle allocation.
l_out_err long_out_parse(struct long_out_s *long_out, char *f_name, \
    struct stat *f_stat);

// Helper functions for long_out_parse
// Fills mode_str with a fixed-size character representation of mode.
void parse_mode_str(char *mode_str, mode_t mode);

// Helper function for long_out_parse
// Finds a user name for uid.
l_out_err parse_usr_str(char **usr_str, uid_t uid);

// Helper function for long_out_parse
// Finds a group name for gid.
l_out_err parse_grp_str(char **grp_str, gid_t gid);

// Helper function for long_out_parse
// Finds the formatted date for mtim.
l_out_err parse_mtim_str(char *mtim_str, time_t mtim);

// Helper function for long_out_parse
// Parses all members of f_stat to strings who need no special formatting.
l_out_err parse_misc_st_str(struct long_out_s *long_out, struct stat *f_stat);

// Prints all entries in dir_long_out using long-format output.
void dir_long_out_print(struct dir_long_out_s *dir_long_out, bool option_i);

// Deletes all the long_out entries in dir_long_out and then zeroes/NULLs
//   the rest of the entries.
void dir_long_out_delete(struct dir_long_out_s *dir_long_out);

// Frees all elements of long_out.
void long_out_delete(struct long_out_s *long_out);

// Gets the char representation of the filetype specified by mode.
char get_type_char(mode_t mode);

#endif /* __LONG_OUT_H */