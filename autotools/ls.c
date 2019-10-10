#include "ls.h"

#define IS_OPT(x) (x[0]=='-')

/* Parses options, finds the first valid path to a directory and prints all the entries
 *   of that directory.
 * TODO: Loop through more directories after the first
 */
int main(int argc, char **argv) {
    DIR *d;
    tree *list = bst_init();
    char options;
    unsigned int path_ind;
    char *path, *err_str;
    int err;

    if (list == NULL) {
        perror("bst_init");
        fprintf(stderr, "exiting...\n");
        return -1;
    }

    options = get_options(argc, argv);

    path_ind = argc-1;
    if (argc == 1 || IS_OPT(argv[path_ind]))
        path = ".";
    else
        path = argv[path_ind];

    d = opendir(path);
    while (d == NULL) {
        err = errno;

        err_str = malloc(sizeof(char) * (OPENDIR_ERROR_NONF_LEN + strlen(path) + 1));
        sprintf(err_str, OPENDIR_ERROR_F, path);

        errno = err;
        perror(err_str);
        free(err_str);
        
        if (path_ind > 1 && !IS_OPT(argv[path_ind-1])) {
            path_ind--;
            path = argv[path_ind];
            d = opendir(path);
            continue;
        }

        fprintf(stderr, "exiting...\n");
        bst_delete_tree_ddata(list);
        return -1;
    }
    
    if (get_dir_listings(d, list, options) < 0) {
        fprintf(stderr, "exiting...\n");
        closedir(d);
        bst_delete_tree_ddata(list);
        return -1;
    }

    bst_inorder_out(list, stdout);
    bst_delete_tree_ddata(list);
    closedir(d);

    return 0;
}

/* Gets flag bits and pushes non-options to the end of argv.
 * The definition of an option in this case is consistent with the IS_OPT macro.
 */
char get_options(int argc, char **argv) {
    int options = 0;
    char opt = getopt(argc, argv, OPT_STRING);

    while (opt != -1) {
        switch (opt) {
        case 'a':
            options |= OPT_a_MASK;
            break;
        }
        opt = getopt(argc, argv, OPT_STRING);
    }
    return options;
}

/* Gets the names of all the files in directory stream d and places
 *   them in the BST list
 * Returns: 0 if successful, -1 on error (either reading the directory or adding an entry)
 */
int get_dir_listings(DIR *d, tree *list, char options) {
    struct dirent *ent;

    errno = 0;
    ent = readdir(d);
    while (ent != NULL) {
        if (ent->d_name[0] == '.' && !(options & OPT_a_MASK)) {
            errno = 0;
            ent = readdir(d);
            continue;
        }

        if (_add_entry(list, ent->d_name) < 0)
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

/* Copies name and then adds it to the BST list
 * Returns: 0 if successful and -1 on error (either duplicate entries in a directory or memory allocation problems)
 */
int _add_entry(tree *list, char *name) {
    int ret, len = strlen(name) + 1;
    char *str = malloc(sizeof(char) * len);

    if (str == NULL) {
        perror("malloc");
        return -1;
    }
    strncpy(str, name, len);

    ret = bst_add_node(list, str);
    if (ret == 1) {
        fprintf(stderr, "Duplicate directory entry %s found, something is wrong\n", name);
        free(str);
        return -1;
    }
    else if (ret == -1) {
        perror("bst_add_node");
        free(str);
        return -1;
    }

    return 0;
}