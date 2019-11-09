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
#include "ls_defs.h"

// Defines for long_out_s
#define MODE_STR_LEN 11
#define DATE_STR_LEN 13

// Stores relevant values for long-format output
struct long_out_s {
    ino_t i_node;
    char mode_str[MODE_STR_LEN];
    nlink_t nlink;
    char *usr_str;
    char *grp_str;
    off_t size;
    char mtim_str[DATE_STR_LEN];
    char *f_name;
};

// Fills the entries of long_out with data for long-format output.
// long_out must point to an already allocated long_out_s struct.
int long_out_parse(struct long_out_s *long_out, struct stat *f_stat, \
    char *f_name);

// Helper function for parse_stat
// Fills mode_str with a fixed-size character representation of mode.
void parse_mode_str(char *mode_str, mode_t mode);

// Helper function for parse_stat
// Finds a user name for uid.
int parse_usr_str(char **usr_str, uid_t uid);

// Helper function for parse_stat
// Finds a group name for gid.
int parse_grp_str(char **grp_str, gid_t gid);

// Helper function for parse_stat
// Finds the formatted date for mtim.
int parse_mtim_str(char *mtim_str, time_t mtim);

// Prints an ls entry in long-format given long_out
void long_out_print(struct long_out_s *long_out);

// Appropriately frees long_out's elements, leaving the struct itself.
void long_out_delete(struct long_out_s *long_out);

// Gets the char representation of the filetype specified by mode.
char get_type_char(mode_t mode);

#endif /* __LONG_OUT_H */