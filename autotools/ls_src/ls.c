/**
 * The ls program. Lists files in the current working directory. Options modify
 *   the output.
 */
#include <dirent.h>
#include <stdbool.h>
#include <assert.h>
#include <limits.h>
#include "list.h"
#include "long_out.h"
#include "ls_defs.h"

// All valid options for ls
#define OPTION_STRING "adil"

// Option flags. These are ONLY set by the get_options function.

// Show files begining with .
bool option_a = false;
// Output files in long-format
bool option_l = false;
// Output i-node number
bool option_i = false;
// Output just the file specified
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
    LS_ERR_LONG_PARSE = 6
};

// List all the directory entries of path. Outputs the result to standard out.
ls_err ls(char *path);

// Gets all the entries located in path and stores them in dir_entries.
ls_err get_entries(const char *path, list *dir_entries);
ls_err add_entry(char *f_name, const char *path, list *dir_entries);

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

/** 
 * Entry point for ls. Ensures arguments are defined and then calls ls
 *   functionality for all specified file paths. On an error, will attempt to
 *   continue running through the rest of the paths unless an apparent memory
 *   allocation has occured.
 * Returns 0 on success, >0 on error
 */
int main(int argc, char **argv) {
    ls_err err = LS_ERR_NONE;
    int ret = 0;

    ret = get_options(argc, argv);
    if (ret < 0) {
        printf("Usage: %s [-%s]\n", argv[0], OPTION_STRING);
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
        else {
            int j = i;
            while (argv[j] != NULL && argv[j][0] != '-' \
                    && err != LS_ERR_MALLOC) {
                if (j > i) {
                    printf("\n");
                }
                printf("%s:\n", argv[j]);
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
    list dir_entries = NULL;
    ls_err ret = LS_ERR_NONE;

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
    else {
        output_entries(&dir_entries);
    }

    list_delete(&dir_entries);
    return ret;
}

/**
 * Opens and iterates through a directory stream at path, and for each entry
 *   entering it into dir_entries with a call to add_entry.
 * Returns LS_ERR_NONE on success, and any other ls_err otherwise depending
 *   on the type of failure.
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
    }
    closedir(d);
    return ret;
}

/** 
 * Adds a new node containing f_name into dir_entries. If option_l or option_i
 *   are true, it also stores the file's stat struct in dir_entries as well.
 *   If path is NULL, stat is just given f_name. Otherwise f_name is appended to
 *   path.
 * Returns LS_ERR_NONE on success, and any other value of ls_err if an error
 *   occured.
 */
ls_err add_entry(char *f_name, const char *path, list *dir_entries) {
    node *ent_node = NULL;
    struct stat *ent_stat = NULL;
    char stat_path[PATH_MAX];
    ls_err ret = LS_ERR_NONE;

    if (option_l || option_i) {
        if (ret == LS_ERR_NONE) {
            errno = 0;
            ent_stat = malloc(sizeof(struct stat));
            if (ent_stat == NULL) {
                ret = LS_ERR_MALLOC;
            }
            else if (path != NULL) {
                ret = get_full_path(stat_path, PATH_MAX, f_name, path);
                if (ret == LS_ERR_NONE && lstat(stat_path, ent_stat) < 0) {
                    free(ent_stat);
                    ret = LS_ERR_STAT;
                }
            }
            else if (lstat(f_name, ent_stat) < 0) {
                free(ent_stat);
                ret = LS_ERR_STAT;
            }
        }
    }

    if (ret == LS_ERR_NONE) {    
        ent_node = list_create_node(f_name, ent_stat);
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
 * Outputs the filenames stored in dir_entries to stdout
 */
void output_entries(list *dir_entries) {
    node *curr;
    curr = *dir_entries;
    while (curr != NULL) {
        if (option_i) {
            printf(INO_PRINTF" ", curr->data.f_stat->st_ino);
        }
        printf("%s\n", curr->data.f_name);
        curr = curr->next;
    }
}

/**
 * Outputs the filenames and stat information in dir_entries to stdout. asserts
 *   that each entry has a non-null f_stat field.
 * Returns LS_ERR_NONE on success, and LS_ERR_LONG_PARSE if the long-format
 *   output could not be parsed.
 */
ls_err output_entries_long(list *dir_entries) {
    struct long_out_s long_out;
    node *curr;
    int err = 0;
    ls_err ret = LS_ERR_NONE;
    
    curr = *dir_entries;
    while (curr != NULL && ret == LS_ERR_NONE) {
        assert(curr->data.f_stat != NULL);

        err = long_out_parse(&long_out, curr->data.f_stat, curr->data.f_name);
        if (err < 0) {
            ret = LS_ERR_LONG_PARSE;
        }
        else {
            long_out_print(&long_out);
            long_out_delete(&long_out);
            curr = curr->next;
        }
    }
    return ret;
}

/**
 * Checks argv for options and sets option flags.
 * Returns 0 on success, -1 if an invalid option was found.
 */
int get_options(const int argc, char **argv) {
    char opt = -1;
    int ret = 0;
    
    opt = getopt(argc, argv, OPTION_STRING);
    while (opt != -1) {
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
        opt = getopt(argc, argv, OPTION_STRING);
    }
    return ret;
}

/**
 * Outputs any errors using each err's corresponding ls_err_message entry.
 *   In standard linux fashion, the program's name is prefixed to the error 
 *   output for clearer messages when chains of programs are piped into one
 *   another.
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
    };
    char str_buf[4096];
    int errno_temp = errno;

    if (errno_temp) {
        memcpy(str_buf, pname, strlen(pname) + 1);
        memcpy(str_buf + strlen(pname), ls_err_msg[err], \
            strlen(ls_err_msg[err]) + 1);
    
        errno = errno_temp;
        perror(str_buf);
    }
    else {
        fprintf(stderr, "%s%s", pname, ls_err_msg[err]);
    }
}