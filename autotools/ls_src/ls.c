/**
 * The ls program. Lists files in the current working directory. Options modify
 *   the output.
 */
#include <dirent.h>
#include <stdbool.h>
#include <assert.h>
#include <limits.h>
#include <termios.h>
#include <sys/ioctl.h>
#include "list.h"
#include "long_out.h"
#include "print_utils.h"

// All valid options for ls
#define OPTION_STRING "adil"

// Option flags. These are ONLY set by the get_options function.

// Show files begining with .
bool option_a = false;
// Output files in long-format
bool option_l = false;
// Output i-node number
bool option_i = false;
// Output just the file specified, don't open it as a directory
bool option_d = false;

// Error definitions
typedef enum ls_err ls_err;
enum ls_err {
    LS_ERR_NONE = 0,
    LS_ERR_MALLOC = 1,
    LS_ERR_DIR_STREAM_OPEN = 2,
    LS_ERR_DIR_STREAM_READ = 3,
    LS_ERR_STAT = 4,
    LS_ERR_PATH_OVERFLOW = 5,
    LS_ERR_LONG_PARSE = 6,
    LS_ERR_IOCTL_TTY = 7
};

// List all the directory entries of path. Outputs the result to standard out.
ls_err ls(char *path);

// Gets all the entries located in path and stores them in dir_entries.
ls_err get_entries(const char *path, list *dir_entries);

// Adds the file f_name at path to dir_entries. If path is NULL it is ignored.
//   dir_entries must already be allocated.
ls_err add_entry(char *f_name, const char *path, list *dir_entries);

// Helper functions for getting a files stat struct.
ls_err get_f_stat(char *f_name, const char *path, struct stat **f_stat);
ls_err get_full_path(char *path_buf, int path_buf_len, const char *f_name, \
    const char *path);

// Outputs all entries of dir_entries to stdout.
void output_entries(list *dir_entries);

// Column based output when printing to a terminal.
ls_err output_entries_tty(list *dir_entries);

// Helper function for output_entries_tty.
char** get_tty_out(list *dir_entries);

// Output all entries of dir_entries to stdout using the long-format output.
ls_err output_entries_long(list *dir_entries);

// Sets the option flags given an array of arguments and their size.
int get_options(const int argc, char **argv);

// Basic error output for ls.
void ls_perror(ls_err err, char *pname);

/** 
 * Entry point for ls. Sets argument flags and then calls ls for all specified
 *   file paths. On an error, will print the error and attempt to continue
 *   running any additional paths unless a memory allocation error has occured.
 * Returns 0 on success, >0 on error.
 */
int main(int argc, char **argv) {
    ls_err err = LS_ERR_NONE;
    int ret = 0;

    ret = get_options(argc, argv);
    if (ret < 0) {
        printf("Usage: %s [-%s] [paths...]\n", argv[0], OPTION_STRING);
    }
    else {
        int i = 1;
        while (argv[i] != NULL && argv[i][0] == '-') {
            i++;
        }
        if (argv[i] == NULL) {
            err = ls(".");
            if (err != LS_ERR_NONE) {
                ls_perror(err, argv[0]);
                ret = err;
            }
        }
        else if (argv[i+1] == NULL) {
            err = ls(argv[i]);
            if (err != LS_ERR_NONE) {
                ls_perror(err, argv[0]);
                ret = err;
            }
        }
        else {
            int j = i;
            while (argv[j] != NULL && err != LS_ERR_MALLOC) {
                if (argv[j][0] == '-') {
                    printf("Can't open directory %s, ")
                }
                if (j > i) {
                    printf("\n");   
                }
                if (!option_d) {
                    printf("%s:\n", argv[j]);
                }
                err = ls(argv[j]);
                if (err != LS_ERR_NONE) {
                    ls_perror(err, argv[0]);
                    ret = err;
                }
                j++;
            }
        }
    }

    return ret;
}

/**
 * Prints the entries of the directory specified by path to stdout.
 * Returns LS_ERR_NONE on success, and on error an ls_err value corresponding
 *   to the type of error.
 */
ls_err ls(char *path) {
    list dir_entries;
    ls_err ret = LS_ERR_NONE;

    list_init(&dir_entries);
    if (option_d) {
        ret = add_entry(path, NULL, &dir_entries);
    }
    else {
        ret = get_entries(path, &dir_entries);
    }
    if (ret != LS_ERR_NONE) {
        list_delete(&dir_entries);
    }
    else if (option_l) {
        ret = output_entries_long(&dir_entries);
    }
    else if (isatty(STDOUT_FILENO)) {
        ret = output_entries_tty(&dir_entries);
    }
    else {
        output_entries(&dir_entries);
    }

    list_delete(&dir_entries);
    return ret;
}

/**
 * Opens and iterates through a directory stream at path, and for each entry
 *   entering it into dir_entries with a call to add_entry.
 * Returns LS_ERR_NONE on success, and on error an ls_err value corresponding
 *   to the type of error.
 */
ls_err get_entries(const char *path, list *dir_entries) {
    DIR *d = NULL;
    struct dirent *ent = NULL;
    ls_err ret = LS_ERR_NONE;

    errno = 0;
    d = opendir(path);
    if (d == NULL) {
        ret = LS_ERR_DIR_STREAM_OPEN;
    }
    else {
        errno = 0;
        ent = readdir(d);
        while (ent != NULL && ret == LS_ERR_NONE) {
            if (option_a || ent->d_name[0] != '.') {
                ret = add_entry(ent->d_name, path, dir_entries);
            }
            errno = 0;
            ent = readdir(d);
        }
        if (ent == NULL && errno) {
            ret = LS_ERR_DIR_STREAM_READ;
        }
        closedir(d);
    }
    return ret;
}

/** 
 * Adds a new node containing f_name into dir_entries. If option_l or option_i
 *   are true, it also stores the file's stat struct in dir_entries as well.
 * Returns LS_ERR_NONE on success, and on error an ls_err value corresponding
 *   to the type of error.
 */
ls_err add_entry(char *f_name, const char *path, list *dir_entries) {
    node *ent_node = NULL;
    struct stat *f_stat = NULL;
    ls_err ret = LS_ERR_NONE;

    if (option_l || option_i) {
        ret = get_f_stat(f_name, path, &f_stat);
    }

    if (ret == LS_ERR_NONE) {
        ent_node = list_create_node(f_name, f_stat);
        if (ent_node == NULL) {
            ret = LS_ERR_MALLOC;
        }
        else {
            list_insert_ordered(dir_entries, ent_node);
        }
    }
    return ret;
}

/**
 * Allocates and gets the stat struct for f_name at path. If path is NULL, it
 *   is ignored and f_name is passed by itself to lstat.
 * Returns LS_ERR_NONE on success, and on error an ls_err value corresponding
 *   to the type of error.
 */
ls_err get_f_stat(char *f_name, const char *path, struct stat **f_stat) {
    char stat_path[PATH_MAX];
    ls_err ret = LS_ERR_NONE;

    errno = 0;
    *f_stat = malloc(sizeof(struct stat));
    if (*f_stat == NULL) {
        ret = LS_ERR_MALLOC;
    }
    else if (path != NULL) {
        ret = get_full_path(stat_path, PATH_MAX, f_name, path);
        if (ret == LS_ERR_NONE && lstat(stat_path, *f_stat) < 0) {
            free(*f_stat);
            ret = LS_ERR_STAT;
        }
    }
    else if (lstat(f_name, *f_stat) < 0) {
        free(*f_stat);
        ret = LS_ERR_STAT;
    }
    return ret;
}

/** 
 * Gets the full path of f_name given path and a buffer to store the 
 *   new path.
 * Returns LS_ERR_NONE on success and LS_ERR_PATH_OVERFLOW if the concatenated
 *   path would overflow path_buf.
 */
ls_err get_full_path(char *path_buf, int path_buf_len, const char *f_name, \
        const char *path) {
    ls_err ret = LS_ERR_NONE;

    if (strlen(path) + strlen(f_name) + 2 > path_buf_len) {
        ret = LS_ERR_PATH_OVERFLOW;
    }
    else {
        strncpy(path_buf, path, strlen(path) + 1);
        if (path_buf[strlen(path)-1] != '/') {
            path_buf[strlen(path)+1] = '\0';
            path_buf[strlen(path)] = '/';
        }
        strncpy(path_buf + strlen(path_buf), f_name, strlen(f_name) + 1);
    }

    return ret;
}

/**
 * Outputs basic new line seperated output of all files in dir_entries to
 *   stdout. If option_i is true, their i-node values are printed as well.
 */
void output_entries(list *dir_entries) {
    node *curr;

    curr = dir_entries->head;
    while (curr != NULL) {
        if (option_i) {
            printf(INO_PRINTF" ", curr->data.f_stat->st_ino);
        }
        printf("%s\n", curr->data.f_name);
        curr = curr->next;
    }
}

/**
 * Output of all files in dir_entries to stdout when stdout is a terminal. If
 *   option_i is true, their i-node values are printed as well.
 * Files are placed in columns based on the size of the largest entry to be
 *   printed.
 * Returns LS_ERR_NONE on success, LS_ERR_MALLOC if memory allocation or the
 *   LS_ERR_IOCTL_TTY if the query for the terminal's size attributes failed.
 */
ls_err output_entries_tty(list *dir_entries) {
    char **tty_out = NULL;
    int max_ent_len, max_col_width, col_num;
    static const int min_col_space = 3;
    struct winsize wsize;
    ls_err ret = LS_ERR_NONE;

    tty_out = get_tty_out(dir_entries);
    if (tty_out == NULL) {
        ret = LS_ERR_MALLOC;
    }
    else if (tty_out[0] != NULL) {
        max_ent_len = strlen(tty_out[0]);
        for (int i = 1; i < dir_entries->size; i++) {
            if (strlen(tty_out[i]) > max_ent_len) {
                max_ent_len = strlen(tty_out[i]);
            }
        }

        max_col_width = max_ent_len + min_col_space;
        errno = 0;
        if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &wsize) < 0) {
            ret = LS_ERR_IOCTL_TTY;
        }
        else {
            col_num = wsize.ws_row / max_col_width;
            int col_curr = 0;
            for (int i = 0; i < dir_entries->size; i++) {
                if (col_curr == col_num) {
                    printf("\n");
                    col_curr = 0;
                }
                printf("%s%*s", tty_out[i], \
                    max_col_width - (int)strlen(tty_out[i]), "");
                col_curr++;
            }
            printf("\n");
        }
    }
    else {
        printf("\n");
    }
    return ret;
}

/**
 * Creates an array of strings for terminal output in output_entries_tty. If
 *   option_i is true, the i-node is prepended to the file name.
 * Returns the array on success and NULL if memory allocation failed.
 */
char** get_tty_out(list *dir_entries) {
    node *curr = NULL;
    char **tty_out;
    int err = 0;

    errno = 0;
    tty_out = malloc(sizeof(char*) * dir_entries->size);
    if (tty_out != NULL) {
        curr = dir_entries->head;
        int i = 0;
        while (err == 0 && curr != NULL && i < dir_entries->size) {
            if (option_i) {
                errno = 0;
                tty_out[i] = malloc(strlen(curr->data.f_name) + \
                    get_f_max_strlen(INO_PRINTF) + 1);
                if (tty_out[i] == NULL) {
                    for (int j = 0; j < i; j++) {
                        free(tty_out[j]);
                    }
                    err = -1;
                }
                else {
                    sprintf(tty_out[i], INO_PRINTF" ", \
                        curr->data.f_stat->st_ino);
                    strncpy(tty_out[i] + strlen(tty_out[i]), \
                        curr->data.f_name, strlen(curr->data.f_name) + 1);
                }
            }
            else {
                tty_out[i] = curr->data.f_name;
            }
            curr = curr->next;
            i++;
        }
        if (err == 0) {
            assert(curr == NULL && i == dir_entries->size);
        }
        else {
            free(tty_out);
            tty_out = NULL;
        }
    }
    return tty_out;
}

/**
 * Outputs the filenames and stat information in dir_entries to stdout. If
 *   option_i is true, their i-node values are printed as well.
 * Returns LS_ERR_NONE on success, and LS_ERR_LONG_PARSE if the long-format
 *   output could not be parsed.
 */
ls_err output_entries_long(list *dir_entries) {
    struct dir_long_out_s dir_long_out;
    l_out_err err = 0;
    ls_err ret = LS_ERR_NONE;
    
    dir_long_out_init(&dir_long_out);
    err = dir_long_out_create(&dir_long_out, dir_entries);
    if (err == L_OUT_ERR_MALLOC) {
        ret = LS_ERR_MALLOC;
    }
    else if (err == L_OUT_ERR_PARSE) {
        ret = LS_ERR_LONG_PARSE;
    }
    else {
        dir_long_out_print(&dir_long_out, option_i);
        dir_long_out_delete(&dir_long_out);
    }
    return ret;
}

/**
 * Checks argv for options and sets option flags.
 * Returns 0 on success, -1 if an invalid option was found.
 */
int get_options(const int argc, char **argv) {
    char opt;
    int ret = 0;
    
    opt = 0;
    while (opt != -1 && ret != -1) {
        opt = getopt(argc, argv, OPTION_STRING);
        switch (opt) {
        case 'a':
            option_a = true;
            break;
        case 'd':
            option_d = true;
            break;
        case 'i':
            option_i = true;
            break;
        case 'l':
            option_l = true;
            break;
        case '?':
            ret = -1;
        }
    }
    return ret;
}

/**
 * Outputs any errors using each err's corresponding ls_err_message entry.
 *   In standard linux fashion, the program's name is prefixed to the error 
 *   output for clearer messages when chains of programs are piped into one
 *   another.
 * Since this is the only function that uses the values of ls_err_msg, its
 *   included as a static array that must have the same number of elements
 *   as the ls_err enum.
 * If errno is set, this prints using perror. This errno value should be 
 *   associated with the err actually given, as almost all the ls_err's are
 *   fatal and instantly break out to main with no subsequent system or 
 *   library calls that would reset errno.
 */
void ls_perror(ls_err err, char *pname) {
    static const char *const ls_err_msg[] = {
        ": no error",
        ": memory allocation error",
        ": error opening a directory stream",
        ": error reading a directory stream",
        ": error getting file statistics",
        ": error converting path: overflow detected",
        ": error parsing long-format output"
        ": error calling ioctl_tty"
    };
    char str_buf[4096];
    int errno_temp = errno;

    if (errno_temp) {
        strncpy(str_buf, pname, strlen(pname) + 1);
        strncpy(str_buf + strlen(pname), ls_err_msg[err], \
            strlen(ls_err_msg[err]) + 1);
    
        errno = errno_temp;
        perror(str_buf);
    }
    else {
        fprintf(stderr, "%s%s", pname, ls_err_msg[err]);
    }
}
