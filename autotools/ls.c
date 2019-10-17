#include "ls.h"

// Quick macro to help in navigating argv
#define IS_OPT(s) (s[0]=='-')

/* Parses options, and in the spirit of the real ls parses all non-option
 *   arguments as paths to sequentially evaluate. The sequence of paths is 
 *   evaluated with FIFO order.
 * Returns: 0 on success, 1 on error.
 */
int main(int argc, char **argv) {
    char options;
    unsigned int path_ind;
    bool cont;

    int err, ret = 0;

    options = get_options(argc, argv);
    if (options == -1) {
        printf("Usage: ls [-a] [file...]\n");
        return 1;
    }

    ls_err_prog = argv[0];

    for (path_ind = 1; path_ind<argc && IS_OPT(argv[path_ind]); path_ind++) continue;
    if (path_ind == argc) {
        ret = _ls(".", options);
    }
    else if (path_ind == argc-1) {
        ret = _ls(argv[path_ind], options);
        if (ret && ls_err_state != LS_ERR_NONE) {
            ls_perror();
        }
    }
    else {
        do {
            printf("%s:\n", argv[path_ind]);

            err = _ls(argv[path_ind], options);
            if (err) {
                ret |= err;
                if (ls_err_state != LS_ERR_NONE) {    
                    ls_perror();
                    if (ls_err_state == LS_ERR_MALLOC) break;
                }
            }
            path_ind++;
            cont = path_ind < argc;
            
            if (cont) printf("\n");
        } while (cont);
    }

    return ret;
}

/* For printing ls errors with meaningfull errno values. Prints the program name, the error 
 *   message indexed by ls_error_state, and passes that string to perror to be handled by
 *   perror 
 */
void ls_perror() {
    char buffer[4096];
    int err = errno;
    if (ls_err_state == LS_ERR_MALLOC) {
        sprintf(buffer, "%s: %s", ls_err_prog, ls_err_str[ls_err_state]);
    }
    else if (ls_err_state == LS_ERR_DIR_ACC || ls_err_state == LS_ERR_DIR_READ_ENTRY \
             || ls_err_state == LS_ERR_LSTAT) {
        sprintf(buffer, "%s: ", ls_err_prog);
        sprintf(buffer + strlen(buffer), ls_err_str[ls_err_state], ls_err_path);
    }
    errno = err;
    perror(buffer);
}

/* Prints the entries of a single path to stdout with a given set of options
 * Returns: 0 on success, 1 on error
 */
int _ls(char *path, char options) {
    DIR *d;
    f_list dir_entries;
    int ret, err;

    errno = 0;
    d = opendir(path);
    if (d == NULL) {
        ls_err_state = LS_ERR_DIR_ACC;
        ls_err_path = path;
        return 1;
    }

    if (f_list_init(&dir_entries) == NULL) {
        ls_err_state = LS_ERR_MALLOC;
        return 1;
    }

    if (options & OPT_l_MASK)
        ret = get_ent_stats(d, &dir_entries, path, options);
    else 
        ret = get_ent_names(d, &dir_entries, path, options);

    if (ret) {
        switch (ls_err_state) {
        case LS_ERR_NONE:
            break;
        case LS_ERR_MALLOC:
            goto err_list_cleanup;
        case LS_ERR_DIR_READ_ENTRY:
            goto err_list_cleanup;
        case LS_ERR_LSTAT:
            goto err_list_cleanup;
        }
        switch (dir_entries.err) {
        case FL_ERR_NONE:
            break;
        case FL_ERR_MALLOC:
            ls_err_state = LS_ERR_MALLOC;
            goto err_list_cleanup;
        case FL_ERR_ARRAY_OVERFLOW:
            fprintf(stderr, "%s\n", f_list_err_out[FL_ERR_ARRAY_OVERFLOW]);
            fprintf(stderr, "printing entries available\n");
            break;
        }
    }
   
    closedir(d);

    f_list_sort(&dir_entries);
    // TODO: Change with actual out functions
    f_list_data_out(&dir_entries, stdout);
    return ret;

    err_list_cleanup:
    err = errno;
    f_list_delete_data(&dir_entries);
    errno = err;
    return ret;
    
}

/* Gets flag bits and pushes non-options to the end of argv.
 * Each character in a string that starts with a - is considered an option.
 * Returns: 0 on success, -1 if an unknown option is found.
 */
char get_options(int argc, char **argv) {
    int options = 0;
    char opt = getopt(argc, argv, OPT_STRING);

    while (opt != -1) {
        switch (opt) {
        case 'a':
            options |= OPT_a_MASK;
            break;
        case 'l':
            options |= OPT_l_MASK;
            break;
        case '?':
            return -1;
        }
        opt = getopt(argc, argv, OPT_STRING);
    }
    return options;
}

/* Gets the names of all the files in directory stream d and places
 *   them in dir_entries
 * Returns: 0 if successful, 1 on error
 */
int get_ent_names(DIR *d, f_list *dir_entries, const char *path, const char options) {
    struct dirent *ent;

    errno = 0;
    ent = readdir(d);
    while (ent != NULL) {
        if (ent->d_name[0] == '.' && !(options & OPT_a_MASK)) {
            errno = 0;
            ent = readdir(d);
            continue;
        }

        if (f_list_add_elem(dir_entries, ent->d_name, NULL)) {
            if (dir_entries->err != FL_ERR_NONE)
                return 1;
        }

        errno = 0;
        ent = readdir(d);
    }
    if (errno != 0) {
        ls_err_state = LS_ERR_DIR_READ_ENTRY;
        ls_err_path = path;
        return 1;
    }
    return 0;
}

/* Gets the names and stat structures of all the files in the directory stream
 *   and places them in dir_entries
 * Returns: 0 if successful, 1 on error
 */
int get_ent_stats(DIR *d, f_list *dir_entries, const char *path, const char options) {
    struct dirent *ent;
    struct stat *ent_stat;
    char path_buf[PATH_MAX];
    int path_len = strlen(path), err;

    memcpy(path_buf, path, path_len + 1);
    if (path_buf[path_len-1] != '/') {
        path_buf[path_len] = '/';
        path_len++;
    }

    errno = 0;
    ent = readdir(d);
    while (ent != NULL) {
        if (ent->d_name[0] == '.' && !(options & OPT_a_MASK)) {
            errno = 0;
            ent = readdir(d);
            continue;
        }
        
        errno = 0;
        ent_stat = malloc(sizeof(struct stat));
        if (ent_stat == NULL) {
            ls_err_state = LS_ERR_MALLOC;
            return 1;
        }

        memcpy(path_buf + path_len, ent->d_name, strlen(ent->d_name) + 1);
        errno = 0;
        if (lstat(path_buf, ent_stat) < 0) {
            err = errno;
            free(ent_stat);
            errno = err;
            ls_err_state = LS_ERR_LSTAT;
            ls_err_path = path_buf; // File path
            return 1;
        }

        if (f_list_add_elem(dir_entries, ent->d_name, ent_stat)) {
            if (dir_entries->err != FL_ERR_NONE)
                return 1;
        }

        errno = 0;
        ent = readdir(d);
    }
    if (errno != 0) {
        ls_err_state = LS_ERR_DIR_READ_ENTRY;
        ls_err_path = path; // Directory path
        return 1;
    }
    return 0;
}