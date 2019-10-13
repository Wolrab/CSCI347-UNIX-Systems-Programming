#include "ls.h"

// TODO: Scrap all error checking and remake it from scratch
// Return error is an enum value to an array?
// Also, using argv[0] like big boi ls

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
    int err, ret = 0;
    bool cont;

    options = get_options(argc, argv);
    if (options == -1) {
        printf("Usage: ls [-a] [file...]\n");
        return 1;
    }

    for (path_ind = 1; path_ind<argc && IS_OPT(argv[path_ind]); path_ind++) continue;
    if (path_ind == argc) {
        ret = _ls(".", options);
    }
    else if (path_ind == argc-1) {
        ret = _ls(argv[path_ind], options);
    }
    else {
        do {
            printf("%s:\n", argv[path_ind]);

            err = _ls(argv[path_ind], options);
            if (!ret && err) ret = err;
            
            path_ind++;
            cont = path_ind < argc;
            
            if (cont) printf("\n");
        } while (cont);
    }

    return ret;
}

/* Prints the entries of a single path to stdout with a given set of options
 * Returns: 0 on success, 1 on error (printing the errors when past functions have not taken care of that)
 */
int _ls(char *path, char options) {
    DIR *d;
    f_list *dir_entries;
    int ret, err;
    char *err_str;

    d = opendir(path);
    if (d == NULL) {
        err = errno;

        err_str = malloc(sizeof(char) * (OPENDIR_ERROR_NONF_LEN + strlen(path) + 1));
        sprintf(err_str, OPENDIR_ERROR_F, path);

        errno = err;
        perror(err_str);
        free(err_str);
        return 1;
    }

    dir_entries = f_list_init();    
    if (dir_entries == NULL) {
        perror("f_list_init");
        return 1;
    }

    if (options & OPT_l_MASK) {
        ret = get_ent_stats(d, dir_entries, path, options);
        if (ret == 2) return 1;
    }
    else
        ret = get_ent_names(d, dir_entries, options);

    if (ret < 0) {
        if (errno == 0)
            fprintf(stderr, "Attempting to print directories saved\n\n");
        else {
            perror("This is fucked");
            f_list_delete_ddata(dir_entries);
            return 1;
        }
    }
    else if (ret == 1) {
        perror("readdir");
        if (dir_entries == NULL) {
            fprintf(stderr, "Catastrophic, exiting.\n");
            return 1;
        }
        else fprintf(stderr, "Attempting to continue.\n\n");
    }
    closedir(d);

    f_list_sort(dir_entries);
    // TODO: Change with actual out functions
    f_list_data_out(dir_entries, stdout);
    f_list_delete_ddata(dir_entries);
    return 0;
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
 * Returns: 0 if successful, 1 on readdir error and -1 for memory related errors
 */
int get_ent_names(DIR *d, f_list *dir_entries, const char options) {
    struct dirent *ent;

    errno = 0;
    ent = readdir(d);
    while (ent != NULL) {
        if (ent->d_name[0] == '.' && !(options & OPT_a_MASK)) {
            errno = 0;
            ent = readdir(d);
            continue;
        }

        if (_add_entry(dir_entries, ent->d_name, NULL) < 0) return -1;

        errno = 0;
        ent = readdir(d);
    }
    if (errno != 0) return 1;
    return 0;
}

/* Gets the names and stat structures of all the files in the directory stream
 *   and places them in dir_entries
 * Returns: 0 if successful, 2 if path is too long, 1 on readdir error and -1 if malloc fails
 */
int get_ent_stats(DIR *d, f_list *dir_entries, const char *path, const char options) {
    struct dirent *ent;
    struct stat *ent_stat;
    char path_buf[4096];
    int path_len = strlen(path);

    if (path_len > 4096 - 256 || (path[path_len-1] != '/' && path_len + 1 > 4096 - 256)) {
        fprintf(stderr, "Pathname is too large for 4096 byte buffer\nSkipping evaluation to prevent buffer overflow\n");
        return 2;
    }

    memcpy(path_buf, path, path_len);
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
        if (ent_stat == NULL) return -1;

        memcpy(path_buf + path_len, ent->d_name, strlen(ent->d_name) + 1);
        errno = 0;
        if (lstat(path_buf, ent_stat) < 0) {
            free(ent_stat);
            return -1;
        }

        if (_add_entry(dir_entries, ent->d_name, ent_stat) < 0) {
            free(ent_stat);
            return -1;
        }

        errno = 0;
        ent = readdir(d);
    }
    if (errno != 0) return 1;
    return 0;
}

/* Copies name and then adds it to the 
 * Returns: 0 if successful and -1 on error (either maximum array size reached or memory allocation problems)
 */
int _add_entry(f_list *dir_entries, char *ent_name, struct stat *ent_stat) {
    int ret, len = strlen(ent_name) + 1;
    char *ent_name_cpy;

    errno = 0;
    ent_name_cpy = malloc(sizeof(char) * len);
    if (ent_name_cpy == NULL) return -1;
    strncpy(ent_name_cpy, ent_name, len);

    ret = f_list_add_elem(dir_entries, ent_name_cpy, ent_stat);
    if (ret == 1) {
        fprintf(stderr, "Reached maximum array size! Are you sure you need to store %u entries?\n", _f_list_size[_F_LIST_SIZE_LEN-1]);
        errno = 0;
        free(ent_name_cpy);
        return -1;
    }
    else if (ret == -1) {
        free(ent_name_cpy);
        return -1;
    }

    return 0;
}