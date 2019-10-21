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

// Global buffers. Static to increase safety.
static char str_buffer[4096];
static const int str_buffer_size = 4096;
static char path_buffer[PATH_MAX];
static const int path_buffer_size = PATH_MAX;

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
    LS_ERR_DIR_OPEN = 2,
    LS_ERR_DIR_READ_ENTRY = 3,
    LS_ERR_STAT = 4,
    LS_ERR_DUP_ENTRY = 5
};
extern const char *const ls_err_str[];
extern const bool ls_err_errno[];

// Global error variables, static because no other compilation unit should
//   be able to change these values.
static ls_err ls_err_state;
static char ls_err_path[PATH_MAX];
static const int ls_err_path_size = PATH_MAX;

// Stores string representations of stat data.
struct stat_out_s {
    char mode_str[11];
    char *usr_str;
    char *grp_str;
    char *mtim_str;
    char *f_name;
};

// List all the directory entries of path. Outputs the result to standard out.
int ls(char *path);

// Stores the entries of the directory specified by path into dir_entries.
int get_entries(const char *path, list *dir_entries);

// Reads all entries of the directory stream d and adds them to dir_entries.
//   Options modify what information is stored.
int read_dir_stream(DIR *d, const char *path, list *dir_entries);

// Stats the file f_name located in the directory referenced by path.
// The stat structure returned must be handled by the user.
struct stat* get_stat(const char *f_name, const char *path);

// Outputs the file names of dir_entries seperate by newlines to stdout.
void output_ent_names(list *dir_entries);

// Output the file names and stat info of dir_entries seperated by newlines to
//   stdout.
int output_ent_stats(list *dir_entries);

// Fills the entries of stat_out with the information in data.
int fill_stat_out(struct stat_out_s *stat_out, struct data_s *data);

// Helper function for fill_stat_out
// Fills mode_str with a fixed-size character representation of mode.
void get_mode_str(char *mode_str, mode_t mode);

// Helper function for fill_stat_out
// Allocates and sets usr_str to either point to a user name associated with 
//   uid, or to a string representation of uid if none is found.
// If errors, guarentees value pointed to by usr_str to be NULL
int get_usr_str(char **usr_str, uid_t uid);

// Helper function for fill_stat_out
// Allocates and sets grp_str to either point to a group name associated with 
//   gid, or to a string representation of gid if none is found.
// If errors, guarentees value pointed to by grp_str to be NULL
int get_grp_str(char **grp_str, gid_t gid);

// Helper function for fill_stat_out
// Allocates and sets mtim_str to be the formatted date representation of mtim,
//   or a filler string if none is found.
// If errors, guarentees value pointed to by mtim_str to be NULL
int get_mtim_str(char **mtim_str, time_t mtim);

// Appropriately frees stat_out elements, leaving the struct itself.
void free_stat_out(struct stat_out_s *stat_out);

// Sets the option flags. Expects raw argc and argv from main.
int get_options(const int argc, char **argv);

// Sets the ls_err values given the current error and the path if relevant, 
//   NULL if it's not.
void set_ls_err(ls_err err, const char *path);

// Clears current errors, necessary before any other errors can be triggered.
void clear_ls_err();

// Prints ls_err errors using the name of the program with reference to the
//   current set error flags and information.
// Prints the errno value through perror if it has meaning in the context of
//   the current error.
void ls_perror(char *program);

#endif /* __LS_H */