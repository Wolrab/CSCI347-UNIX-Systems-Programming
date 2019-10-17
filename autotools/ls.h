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
#include <regex.h>
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
static const char const* opt_string = "al";

// The bitmask for each of the options
// LIMITED TO 7 OPTIONS WITH CHAR as -1 is reserved for an invalid option error
static const char opt_a_mask = 0x01;
static const char opt_l_mask = 0x02;

typedef enum ls_err ls_err;
static enum ls_err {
    LS_ERR_NONE = 0,
    LS_ERR_MALLOC = 1,          // errno: meaningful
    LS_ERR_DIR_ACC = 2,         // errno: meaningful
    LS_ERR_DIR_READ_ENTRY = 3,  // errno: meaningful
    LS_ERR_LSTAT = 4,           // errno: meaningful
    LS_ERR_FOPEN = 5,           // errno: meaningful
    LS_ERR_REGEX = 6            // errno: not meaningful
} ls_err_state = LS_ERR_NONE;
static const char *ls_err_prog;
static const char *ls_err_path;

// TODO: Use to store error values from ahead in the chain
struct ls_err_state_s {
    ls_err err_state;
    const char *err_prog;
    const char *err_path; 
};

static const char *const ls_err_str[] = {
    "no error",
    "malloc",
    "cannot open dir '%s'",
    "cannot read an entry in directory '%s'",
    "cannot stat '%s'"
    "cannot open file '%s'"
};

struct stat_out_s {
    char mode[11];
    char *nlink[10]; // Largest value for a 4 bit signed or unsigned integer
    char *usr;
    char *grp;
    char *size;
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

// If the compiler is epic this will all expand out inside output_ent_stats and the function will
//   secretly be like 100+ lines long
inline int get_stat_out(struct stat_out_s *stat_out, struct stat *f_stat, FILE *passwd, FILE *group);
inline void get_mode(char *mode_s, mode_t mode);
inline void get_nlink(char *nlink_s, nlink_t nlink);
inline int get_usr(char *uid_s, uid_t uid, uid_t gid, FILE *passwd);
inline int get_grp(char *gid_s, uid_t gid, FILE *group);
inline int get_size(char *size_s, off_t size);
inline int get_mtim(char *mtim_s, struct timespec *mtim);

#endif /* __LS_H */