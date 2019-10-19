#ifndef __LS_H
#define __LS_H
#include <dirent.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>
#include "f_list.h"

/* The writable globals for ls are...
 * ls_err ls_err_state
 * static const char* ls_err_prog
 * static const char* ls_err_path
 * 
 * The proper use of these variables must be guarenteed by a VERY strict error-
 *   return path. This is necessary anyways to ensure proper resource deallocation
 *   and errno preservation.
 * 
 * In the future I will create a proper struct that stores all this information so 
 *   that when a function returns out, all the necessary information will be neatly 
 *   contained within recieving the function  
 */

// All the options recognized by the program
const char *const opt_string = "al";

// The bitmask for each of the options
// LIMITED TO 7 OPTIONS WITH CHAR as -1 is reserved for an invalid option error
const char opt_a_mask = 0x01;
const char opt_l_mask = 0x02;

typedef enum ls_err ls_err;
enum ls_err {
    LS_ERR_NONE = 0,
    LS_ERR_MALLOC = 1,          // errno: meaningful
    LS_ERR_DIR_ACC = 2,         // errno: meaningful
    LS_ERR_DIR_READ_ENTRY = 3,  // errno: meaningful
    LS_ERR_LSTAT = 4,           // errno: meaningful
    LS_ERR_LINK_OPEN = 5,       // errno: meaningful
} ls_err_state = LS_ERR_NONE;
const char *ls_err_prog;
const char *ls_err_path;

// TODO: Use to store error values from ahead in the chain
struct ls_err_state_s {
    ls_err err_state;
    const char *err_prog;
    const char *err_path; 
};

const char *const ls_err_str[] = {
    "no error",
    "malloc",
    "cannot open dir '%s'",
    "cannot read an entry in directory '%s'",
    "cannot stat '%s'",
    "cannot open link '%s'"
};

struct stat_out_s {
    char mode[11];     // Unchanging string pattern
    char *usr;
    char *grp;
    char *mtim;
};

// Standardized method for printing error with meaningfull errno's
void ls_perror();

// Actual ls functionality
int _ls(char *path, char options);

// Returns a maskable char with all the enabled options
char get_options(int argc, char **argv);

// Uses _add_entry to fill the f_list with the name of all the entries in d
int get_ent_names(DIR *d, f_list *dir_entries, const char *path, const char options);

// Uses _add_entry to fill the f_list with all the names and stats of entries in d
int get_ent_stats(DIR *d, f_list *dir_entries, const char *path, const char options);

// Outputs the file names of the f_list seperate by newlines
void output_ent_names(f_list *dir_entries);

// Output the file names and statistics of the f_list seperated by newlines
int output_ent_stats(f_list *dir_entries);

// Functionality for output_ent_stats
int get_stat_out(struct stat_out_s *stat_out, struct stat *f_stat);
void get_mode(char *mode_s, mode_t mode);
int get_usr(char **usr, uid_t uid);
int get_grp(char **grp, uid_t gid);
int get_mtim(char **mtim_s, time_t *mtim);
unsigned int get_digits(unsigned long n);

#endif /* __LS_H */