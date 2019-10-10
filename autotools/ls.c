#include "ls.h"

#define IS_ARG(x) (x[0]=='-')

/* Parses arguments, finds the first valid path to a directory and prints all the entries
 *   of that directory
 */
int main(int argc, char **argv) {
    DIR *d;
    tree *list = bst_init();
    int path_ind;
    char *path, *err_str;
    if (list == NULL) {
        perror("bst_init");
        fprintf(stderr, "exiting...\n");
        return -1;
    }

    int flags = get_flags(argc, argv);

    path_ind = argc-1;
    if (argc == 1 || IS_ARG(argv[path_ind]))
        path = ".";
    else
        path = argv[path_ind];

    d = opendir(path);
    while (d == NULL) {
        err_str = malloc(sizeof(char) * (OPENDIR_ERROR_NONF_LEN + strlen(path) + 1));
        sprintf(err_str, OPENDIR_ERROR_F, path);
        perror(err_str);
        free(err_str);
        if (path_ind > 1 && !IS_ARG(argv[path_ind-1])) {
            path_ind--;
            path = argv[path_ind];
            d = opendir(path);
            continue;
        }
        fprintf(stderr, "exiting...\n");
        bst_delete_tree_ddata(list);
        return -1;
    }
    
    if (get_dir_listings(d, list, flags) < 0) {
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

/* Gets flag bits and pushes non-args to the end of argv
 */
int get_flags(int argc, char **argv) {
    int flags = 0;
    char arg = getopt(argc, argv, OPT_STRING);

    while (arg != -1) {
        switch (arg) {
        case 'a':
            flags |= OPT_a_MASK;
            break;
        }
        arg = getopt(argc, argv, OPT_STRING);
    }
    return flags;
}

/* Gets the names of all the files in directory stream d and places
 *   them in the BST list
 */
int get_dir_listings(DIR *d, tree *list, int flags) {
    struct dirent *ent;

    errno = 0;
    ent = readdir(d);
    while (ent != NULL) {
        if (ent->d_name[0] == '.' && !(flags & OPT_a_MASK)) {
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