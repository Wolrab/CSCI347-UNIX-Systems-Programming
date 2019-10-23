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
#include "list.h"

#define OPTION_STRING "al"

// Option flags
static bool option_a = false;
static bool option_l = false;

// Error definitions that index into corresponding error strings and whether
//   a meaningful errno value is set by the error.
typedef enum ls_err ls_err;
enum ls_err {
    LS_ERR_NONE = 0,
    LS_ERR_MALLOC = 1,
    LS_ERR_DIR_STREAM_OPEN = 2,
    LS_ERR_DIR_STREAM_READ = 3,
    LS_ERR_STAT = 4,
    LS_ERR_DUP_ENTRY = 5,
    LS_ERR_PATH_OVERFLOW = 6,
    LS_ERR_DATE_OVERFLOW = 7,
    LS_ERR_USR_NOT_FOUND = 8,
    LS_ERR_GRP_NOT_FOUND = 9
};
extern const char *const ls_err_msg[];

#define MODE_STR_LEN 11
#define DATE_STR_LEN 13

// Stores string representations of stat data.
struct stat_out_s {
    char mode_str[MODE_STR_LEN];
    char *usr_str;
    char *grp_str;
    char mtim_str[DATE_STR_LEN];
    char *f_name;
};

// List all the directory entries of path. Outputs the result to standard out.
ls_err ls(char *path);

/* TODO: comment */
ls_err get_entries(const char *path, list *dir_entries);

/* TODO: comment */
ls_err parse_entry(struct dirent *ent, const char *path, \
        list *dir_entries);

// Gets a files full path from f_name and the path of the directory that 
//   contains it.
ls_err get_full_path(char *path_buf, int path_buf_len, const char *f_name, \
    const char *path);

// Outputs the file names of dir_entries seperate by newlines to stdout.
void output_ent_names(list *dir_entries);

// Output the file names and stat info of dir_entries seperated by newlines to
//   stdout.
ls_err output_ent_stats(list *dir_entries);

// Fills the entries of stat_out with the information in data.
ls_err fill_stat_out(struct stat_out_s *stat_out, struct data_s *data);

// Helper function for fill_stat_out
// Fills mode_str with a fixed-size character representation of mode.
void get_mode_str(char *mode_str, mode_t mode);

// Helper function for fill_stat_out
// Allocates and sets usr_str to either point to a user name associated with 
//   uid, or to a string representation of uid if none is found.
// If errors, guarentees value pointed to by usr_str to be NULL
ls_err get_usr_str(char **usr_str, uid_t uid);

// Helper function for fill_stat_out
// Allocates and sets grp_str to either point to a group name associated with 
//   gid, or to a string representation of gid if none is found.
// If errors, guarentees value pointed to by grp_str to be NULL
ls_err get_grp_str(char **grp_str, gid_t gid);

// Helper function for fill_stat_out
// Finds the date from and from mtim sets it in mtim_str 
ls_err get_mtim_str(char *mtim_str, time_t mtim);

// Appropriately frees stat_out elements, leaving the struct itself.
void free_stat_out(struct stat_out_s *stat_out);

// Sets the option flags. Expects raw argc and argv from main.
int get_options(const int argc, char **argv);

// 
void ls_perror(ls_err err, char *pname);

#endif /* __LS_H */