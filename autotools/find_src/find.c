#include "find.h"

/**
 * This main does mostly error checking. Creates the expression, sets the
 *   necessary globals and then calls find to start itterating through the 
 *   file tree.
 * Returns: 0 on success, 1 on error.
 */
int main(int argc, char **argv) {
    char **expr_argv;
    int expr_argc;
    expression_t expression;
    expr_err e_err = EXPR_ERR_NONE;
    find_err f_err = FIND_ERR_NONE;

    if (check_args(argc, argv) < 0) {
        return 1;
    }

    expr_argv = &(argv[2]);
    expr_argc = argc-2;

    e_err = expression_create(&expression, expr_argc, expr_argv);
    if (e_err != EXPR_ERR_NONE) {
        expression_perror(e_err, argv[0]);
        return 1;
    }

    f_err = find(&(argv[1]), &expression);
    if (f_err) {
        find_perror(f_err, argv[0]);
        
    }
    expression_delete(&expression);

    return ret;
}

/**
 * Itterates through a file tree starting at file, evaluating each new path 
 *   with expression and adding to path_list for each path that evaluates to
 *   true.
 * Returns: 0 on success, 1 on error with an associated errno value
 */
find_err find(char **file, expression_t *expression) {
    FTS *file_tree;
    list path_list = NULL;
    find_err ret = FIND_ERR_NONE;

    errno = 0;
    file_tree = fts_open(file, FTS_PHYSICAL, NULL);
    if (file_tree == NULL) {
        return FIND_ERR_FTREE;
    }
    ret = descend_tree(file_tree, expression, &path_list);
    if (ret == FIND_ERR_MALLOC) {
        goto exit;
    }
    output_path_list(&path_list);
    
    exit:
    fts_close(file_tree);
    list_delete(&path_list);
    return ret;
}

/**
 * 
 */
find_err descend_tree(FTS *file_tree, expression_t *expression, list *path_list) {
    FTSENT *entry;
    node *n;
    find_err ret = FIND_ERR_NONE;

    entry = fts_read(file_tree);
    while (entry != NULL) {
        if (entry->fts_info != FTS_DP && \
        expression_evaluate(expression, entry->fts_statp)) {
            n = list_create_node(entry->fts_path);
            if (n == NULL) {
                return FIND_ERR_MALLOC;
            }
            list_insert_ordered(path_list, n);
        }

        entry = fts_read(file_tree);
    }
    return ret;
}

/**
 * Simple output of path_list to stdout.
 */
void output_path_list(list *path_list) {
    node *curr = *path_list;
    while (curr != NULL) {
        printf("%s\n", curr->data.path);
        curr = curr->next;
    }
}

/**
 * Does basic argument validity checking and prints any errors.
 * Returns: 0 on success, -1 if arguments are invalid.
 */
int check_args(int argc, char **argv) {
    if (argc == 1) {
        printf("%s: invalid arguments\n", argv[0]);
        printf("Usage: %s file [expression]\n", argv[0]);
        return -1;
    }

    errno = 0;
    if (access(argv[1], F_OK) < 0) {
        perror(argv[0]);
        return -1;
    }
    return 0;
}

/**
 * Basic error output for expression creation.
 */
void expression_perror(expr_err err, char *pname) {
    switch(err) {
    case EXPR_ERR_MALLOC:
        perror(pname);
        break;
    case EXPR_ERR_PRIMARY:
        fprintf(stderr, "%s: invalid primary\n", \
            pname);
        break;
    case EXPR_ERR_ARG:
        fprintf(stderr, "%s: invalid argument\n", pname);
        break;
    case EXPR_ERR_NO_ARG:
        fprintf(stderr, "%s: primary missing an argument\n", pname);
        break;
    }
}

/**
 * Basic error output for find
 */
void find_perror(find_err err, char *pname) {

}