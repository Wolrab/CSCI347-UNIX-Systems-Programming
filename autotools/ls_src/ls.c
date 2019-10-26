/**
 * The ls program. Lists files in the current working directory. Options modify
 *   the output.
 */

#include "ls.h"

// Basic error message defines to make ls_perror easier on the eyes
const char *const ls_err_msg[] = {
    "no error",
    "memory allocation error",
    "error opening a directory stream",
    "error reading a directory stream",
    "error getting file statistics",
    "warning: duplicate entry found",
    "error converting path: overflow detected",
    "error parsing long-format output"
};

/** 
 * Entry point for ls. Ensures arguments are defined and then calls ls
 *   functionality on the CWD. If an error occurs, its message is then printed.
 * Returns: 0 on success, >0 on error
 */
int main(int argc, char **argv) {
    ls_err err = LS_ERR_NONE;
    int ret = 0;

    ret = get_options(argc, argv);
    if (ret < 0) {
        printf("Usage: ls [-%s]\n", OPTION_STRING);
        return ret;
    }

    err = ls(".");
    if (err != LS_ERR_NONE) {
        ls_perror(err, argv[0]);
        ret = err;
    }

    return ret;
}

/**
 * Prints the entries of the directory specified by path to stdout.
 * Returns: LS_ERR_NONE on success, and on error an ls_err value corresponding
 *   to the type of error.
 */
ls_err ls(char *path) {
    list dir_entries = NULL;
    ls_err ret = LS_ERR_NONE;

    ret = get_entries(path, &dir_entries);
    if (ret != LS_ERR_NONE) {
        list_delete(&dir_entries);
        return ret;
    }

    if (option_l) {
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
 *   entering it into dir_entries with a call to parse_entry.
 * Returns: LS_ERR_NONE on success, and any other ls_err otherwise depending
 *   on the type of failure. It should be noted that LS_ERR_DUP_ENTRY is not
 *   fatal, but the value will be passed back as long as a fatal error doesn't
 *   occur before the function finishes.
 */
ls_err get_entries(const char *path, list *dir_entries) {
    DIR *d = NULL;
    struct dirent *ent = NULL;
    ls_err ret = LS_ERR_NONE;

    errno = 0;
    d = opendir(path);
    if (d == NULL) {
        return LS_ERR_MALLOC;
    }

    errno = 0;
    ent = readdir(d);
    while (ent != NULL) {
        if (!(option_a) && ent->d_name[0] == '.') {
            errno = 0;
            ent = readdir(d);
            continue;
        }

        ret = parse_entry(ent, path, dir_entries);
        if (ret != LS_ERR_NONE && ret != LS_ERR_DUP_ENTRY) {
            return ret;
        }

        errno = 0;
        ent = readdir(d);
    }
    if (ent == NULL && errno) {
        ret = LS_ERR_DIR_STREAM_READ;
    }

    closedir(d);
    return ret;
}

/** 
 * Parses the data in ent and adds a new node with the parsed data into 
 *   dir_entries. If long-format output is on, it will also store the extra
 *   stat information in dir_entries as well.
 * Returns: LS_ERR_NONE on success, and any other ls_err otherwise depending
 *   on the type of error.
 */
ls_err parse_entry(struct dirent *ent, const char *path, \
        list *dir_entries) {
    node *ent_node = NULL;

    struct stat *ent_stat = NULL;
    char stat_path[PATH_MAX];

    list_err err = LIST_ERR_NONE;
    ls_err ret = LS_ERR_NONE;

    if (option_l) {
        ret = get_full_path(stat_path, PATH_MAX, ent->d_name, path);
        if (ret == LS_ERR_PATH_OVERFLOW) {
            return ret;
        }

        errno = 0;
        ent_stat = malloc(sizeof(struct stat));
        if (ent_stat == NULL) {
            return LS_ERR_MALLOC;
        }

        if (lstat(stat_path, ent_stat) < 0) {
            free(ent_stat);
            return LS_ERR_STAT;
        }
    }

    ent_node = list_create_node(ent->d_name, ent_stat);
    if (ent_node == NULL) {
        return LS_ERR_MALLOC;
    }

    err = list_insert_ordered(dir_entries, ent_node);
    if (err == LIST_ERR_DUP_ENTRY) {
        ret = LS_ERR_DUP_ENTRY;
    }

    return ret;
}

/** 
 * Gets the full path of f_name given path and a buffer to store the 
 *   new path.
 * Returns: LS_ERR_NONE on success and LS_ERR_PATH_OVERFLOW if the concatenated
 *   path would overflow path_buf.
 */
ls_err get_full_path(char *path_buf, int path_buf_len, const char *f_name, \
        const char *path) {
    ls_err ret = LS_ERR_NONE;

    if (strlen(path) + strlen(f_name) + 1 > path_buf_len) {
        return LS_ERR_PATH_OVERFLOW;
    }

    memcpy(path_buf, path, strlen(path) + 1);
    if (path_buf[strlen(path)-1] != '/') {
        path_buf[strlen(path)+1] = '\0';
        path_buf[strlen(path)] = '/';
    }
    memcpy(path_buf + strlen(path_buf), f_name, strlen(f_name) + 1);

    return ret;
}

/**
 * Outputs the filenames stored in dir_entries to stdout
 */
void output_entries(list *dir_entries) {
    node *curr;
    curr = *dir_entries;
    while (curr != NULL) {
        printf("%s\n", curr->data.f_name);
        curr = curr->next;
    }
}

/**
 * Outputs the filenames and stat information in dir_entries to stdout. asserts
 *   that each entry has a non-null f_stat field.
 * Returns: LS_ERR_NONE on success, and LS_ERR_LONG_PARSE if the long-format
 *   output could not be parsed.
 */
ls_err output_entries_long(list *dir_entries) {
    struct long_out_s long_out;
    node *curr;
    int err = 0;
    ls_err ret = LS_ERR_NONE;
    
    curr = *dir_entries;
    while (curr != NULL) {
        assert(curr->data.f_stat != NULL);

        err = long_out_parse(&long_out, curr->data.f_stat, curr->data.f_name);
        if (err < 0) {
            return LS_ERR_LONG_PARSE;
        }

        long_out_print(&long_out);
        long_out_delete(&long_out);
        curr = curr->next;
    }
    return ret;
}

/**
 * Checks argv for options and sets option flags.
 * Returns: 0 on success, -1 if an invalid option was found.
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
    char str_buf[4096];
    int temp = errno;
    memcpy(str_buf, pname, strlen(pname) + 1);
    memcpy(str_buf + strlen(pname), ls_err_msg[err], \
        strlen(ls_err_msg[err]) + 1);
    if (err) {
        errno = temp;
        perror(str_buf);
    }
    else {
        fprintf(stderr, "%s: %s", pname, str_buf);
    }
}