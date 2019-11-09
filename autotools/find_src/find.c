/**
 * find program. Recursively searches a given file tree, and given an expression
 *   prints all files for which the expression evaluates to true.
 */
#include <stdio.h>
#include "list.h"
#include "expression.h"

typedef enum find_err find_err;
enum find_err {
    FIND_ERR_NONE     = 0,
    FIND_ERR_MALLOC   = 1,
    FIND_ERR_FTREE    = 2,
    FIND_ERR_FTS_READ = 3
};

find_err find(char *file, expression_t *expression);

// Helpers for find
find_err descend_tree(FTS *file_tree, expression_t *expression, list *path_list);
void output_path_list(list *path_list);

// Error printing
void expression_perror(expr_err err, char *pname);
void find_perror(find_err err, char *pname);

/**
 * Creates an expression given input from argv, and then calls find to iterrate
 *   through the file tree and evaluate each file. The file is assumed to be at
 *   argv[1] and the expression starts at argv[2]
 * Returns 0 on success, 1 on error.
 */
int main(int argc, char **argv) {
    char **expr_argv = NULL;
    expression_t expression;
    expr_err e_err = EXPR_ERR_NONE;
    find_err f_err = FIND_ERR_NONE;
    int ret = 0;

    if (argc == 1) {
        printf("%s: invalid arguments\n", argv[0]);
        printf("Usage: %s file [expression]\n", argv[0]);
        ret = 1;
    }
    else {
        expr_argv = &(argv[2]);

        e_err = expression_create(&expression, expr_argv);
        if (e_err != EXPR_ERR_NONE) {
            expression_perror(e_err, argv[0]);
            ret = 1;
        }
        else {
            f_err = find(argv[1], &expression);
            if (f_err != FIND_ERR_NONE) {
                find_perror(f_err, argv[0]);
                ret = 1;
            }
            expression_delete(&expression);
        }
    }
    return ret;
}

/**
 * Implementation of find. Descends the file tree with its root at file and
 *   prints out all files in the tree for which expression evaluates to true.
 *   fts_open takes a NULL-terminated array so the NULL-terminated array
 *   consisting of file and NULL is necessary.
 * Returns FIND_ERR_NONE on success and any other find_err on failure.
 */
find_err find(char *file, expression_t *expression) {
    FTS *file_tree = NULL;
    list path_list = NULL;
    find_err ret = FIND_ERR_NONE;
    char *files[] = {file, NULL};

    errno = 0;
    file_tree = fts_open(files, FTS_PHYSICAL, NULL);
    if (file_tree == NULL) {
        ret = FIND_ERR_FTREE;
    }
    else {
        ret = descend_tree(file_tree, expression, &path_list);
        if (ret == FIND_ERR_NONE) {
            output_path_list(&path_list);
        }
        fts_close(file_tree);
        list_delete(&path_list);
    }
    return ret;
}

/**
 * Descends the file tree, evaluating each file with expression and adding
 *   every file that evaluates to true to path_list.
 * Returns FIND_ERR_NONE on success and FIND_ERR_MALLOC or FIND_ERR_FTS_READ if
 *   malloc or fts_read failed respectively.
 */
find_err descend_tree(FTS *file_tree, expression_t *expression, list *path_list) {
    FTSENT *entry = NULL;
    node *n = NULL;
    find_err ret = FIND_ERR_NONE;

    errno = 0;
    entry = fts_read(file_tree);
    while (entry != NULL && ret == FIND_ERR_NONE) {
        if (entry->fts_info != FTS_DP && 
                expression_evaluate(expression, entry)) {
            n = list_create_node(entry->fts_path);
            if (n == NULL) {
                ret = FIND_ERR_MALLOC;
            }
            else {
                list_insert_ordered(path_list, n);
            }
        }
        
        errno = 0;
        entry = fts_read(file_tree);
    }
    if (entry == NULL && errno) {
        ret = FIND_ERR_FTS_READ;
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
 * Basic error output for expression creation. pname should be argv[0] from
 *   main.
 */
void expression_perror(expr_err err, char *pname) {
    switch(err) {
    case EXPR_ERR_NONE:
        break;
    case EXPR_ERR_MALLOC:
        perror(pname);
        break;
    case EXPR_ERR_GLOBALS:
        fprintf(stderr, "%s: could not get required program state info\n", \
            pname);
    case EXPR_ERR_PRIMARY:
        fprintf(stderr, "%s: invalid primary\n", pname);
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
 * Basic error output for find. pname should be argv[0] from main.
 */
void find_perror(find_err err, char *pname) {
    switch (err) {
    case FIND_ERR_NONE:
        break;
    case FIND_ERR_MALLOC:
        perror(pname);
    case FIND_ERR_FTREE:
        perror(pname);
    case FIND_ERR_FTS_READ:
        perror(pname);
    }
}