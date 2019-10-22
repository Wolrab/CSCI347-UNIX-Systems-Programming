#include "find.h"

/**
 * This main does mostly error checking. Creates the expression, sets the
 *   necessary globals and then calls find to start itterating through the 
 *   file tree.
 * Returns: 0 on success, 1 on error.
 */
int main(int argc, char **argv) {
    char **expr_args;
    int expr_args_size;
    expression_t expression;
    list path_list;

    expr_err err = EXPR_ERR_NONE;
    int ret = 0;

    if (argc == 1) {
        printf("%s: invalid arguments\n", argv[0]);
        printf("Usage: %s file [expression]\n", argv[0]);
        return 1;
    }

    if (access(argv[1], F_OK) < 0) {
        printf("%s: file '%s' not found\n", argv[0], argv[1]);
        return 1;
    }

    expr_args = &(argv[2]);
    expr_args_size = argc-2;

    err = expression_create(&expression, expr_args, expr_args_size);
    if (err != EXPR_ERR_NONE) {
        expression_perror(err, argv, expr_args);
        return 1;
    }

    ret = init_global_expression_states();
    if (ret) {
        perror(argv[0]);
        return 1;
    }

    errno = 0;
    if (list_init(&path_list) == NULL) {
        perror(argv[0]);
        return 1;
    }

    ret = find(&(argv[1]), &expression, &path_list);
    if (ret) {
        perror(argv[0]);
        return 1;
    }
    output_path_list(&path_list);

    return ret;
}

/**
 * Used for initializing globals for evaluating expressions. Must only be 
 *   called once
 * Returns: 0 on success, -1 if clock_gettime throws an error.
 */ 
int init_global_expression_states() {
    struct timespec tm;
    int ret = 0;

    errno = 0;
    ret = clock_gettime(CLOCK_REALTIME, &tm);
    if (ret) {
        return ret;
    }
    set_start_time(tm.tv_sec);
    return ret;
}

/**
 * Itterates through a file tree starting at file, evaluating each new path 
 *   with expression and adding to path_list for each path that evaluates to
 *   true.
 * Returns: 0 on success, 1 on error with an associated errno value
 */
int find(char **file, expression_t *expression, list *path_list) {
    FTS *file_tree;
    FTSENT *entry;
    list_err err = LIST_ERR_NONE;

    errno = 0;
    file_tree = fts_open(file, FTS_PHYSICAL, NULL);
    if (file_tree == NULL) {
        return 1;
    }

    entry = fts_read(file_tree);
    while (entry != NULL) {
        if (entry->fts_info != FTS_DP) {
            if (expression_evaluate(expression, entry->fts_statp)) {
                err = list_add_ordered(path_list, entry->fts_path);
                if (err == LIST_ERR_MALLOC) {
                    return 1;
                }
            }
        }
        entry = fts_read(file_tree);
    }
    fts_close(file_tree);
    return 0;
}

/**
 * Simple output of path_list to stdout
 */
void output_path_list(list *path_list) {
    node *curr = *path_list;
    while (curr != NULL) {
        printf("%s\n", curr->data.path);
        curr = curr->next;
    }
}

/**
 * perror function specifically for managing errors from expression function.
 *   This allows for verbose errors relating to malformed expressions.
 */
void expression_perror(expr_err err, char **argv, char **expr_args) {
    switch(err) {
    case EXPR_ERR_MALLOC:
        perror(argv[0]);
        break;
    case EXPR_ERR_INVALID_PRIMARY:
        fprintf(stderr, "%s: invalid primary '%s' in expression\n", \
            argv[0], expr_args[0]);
        break;
    case EXPR_ERR_INVALID_ARG:
        fprintf(stderr, "%s: invalid argument '%s' for primary '%s' in "
            "expression\n", argv[0], expr_args[1], expr_args[0]);
        break;
    case EXPR_ERR_NO_ARG:
        fprintf(stderr, "%s: no argument found for primary '%s'", \
            argv[0], expr_args[0]);
        break;
    }
}