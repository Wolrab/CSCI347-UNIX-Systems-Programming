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
    f_list *ent_names;
    int err;
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

    ent_names = f_list_init();    
    if (ent_names == NULL) {
        perror("f_list_init");
        return 1;
    }

    if (get_dir_listings(d, ent_names, options) < 0) {
        closedir(d);
        f_list_delete_ddata(ent_names);
        return 1;
    }

    f_list_sort(ent_names);
    f_list_data_out(ent_names, stdout);
    closedir(d);
    f_list_delete_ddata(ent_names);
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
        case '?':
            return -1;
        }
        opt = getopt(argc, argv, OPT_STRING);
    }
    return options;
}

/* Gets the names of all the files in directory stream d and places
 *   them in ent_names for easy sorting later
 * Returns: 0 if successful, -1 on error (either reading the directory or adding an entry)
 */
int get_dir_listings(DIR *d, f_list *ent_names, char options) {
    struct dirent *ent;

    errno = 0;
    ent = readdir(d);
    while (ent != NULL) {
        if (ent->d_name[0] == '.' && !(options & OPT_a_MASK)) {
            errno = 0;
            ent = readdir(d);
            continue;
        }

        if (_add_entry(ent_names, ent->d_name) < 0)
            return -1;

        errno = 0;
        ent = readdir(d);
    }
    if (errno != 0) {
        perror("readdir");
        return -1;
    }
    return 0;
}

/* Copies name and then adds it to the 
 * Returns: 0 if successful and -1 on error (either maximum array size reached or memory allocation problems)
 */
int _add_entry(f_list *ent_names, char *name) {
    int ret, len = strlen(name) + 1;
    char *str = malloc(sizeof(char) * len);

    if (str == NULL) {
        perror("malloc");
        return -1;
    }
    strncpy(str, name, len);

    ret = f_list_add_elem(ent_names, str, NULL);
    if (ret == 1) {
        fprintf(stderr, "Reached maximum array size! Are you sure you need to store %u entries?\n", _f_list_size[_F_LIST_SIZE_LEN-1]);
        free(str);
        return -1;
    }
    else if (ret == 2) {
        fprintf(stderr, "The name string passed into f_list_add_elem was somehow NULL. God help you if you see this\n");
        free(str);
        return -1;
    }
    else if (ret == -1) {
        perror("f_list_add_elem");
        free(str);
        return -1;
    }

    return 0;
}
