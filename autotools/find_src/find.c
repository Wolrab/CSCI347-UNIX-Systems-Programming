/**
 * find program. Recursively searches a given file tree, and given an expression
 *   prints all files that make the expression return true.
 */
#include <stdio.h>
#include "list.h"
#include "expression.h"
typedef enum find_err find_err;

enum find_err {
    FIND_ERR_NONE = 0,
    FIND_ERR_MALLOC = 1,
    FIND_ERR_FTREE = 2
};

// Iterates through the file tree originating from file and evaluating paths 
//   against the expression. Prints all files that the expression evaluated
//   to true once it's done.
find_err find(char *file, expression_t *expression);

// Decends the tree, adding to path_list given the evaluation of expression
//   for each file.
find_err descend_tree(FTS *file_tree, expression_t *expression, list *path_list);

// Simple output to stdout of the list of paths.
void output_path_list(list *path_list);

// Simple argument validator/checker.
int check_args(int argc, char **argv);

// Error output for errors returned from expression_create.
void expression_perror(expr_err err, char *pname);

// Error output for find
void find_perror(find_err err, char *pname);

/**
 * Does basic error checking, creates an expression from the available args,
 *   and then calls find to do the rest. Calls the appropriate *_perror
 *   function if necessary.
 * Returns: 0 on success, 1 on error.
 */
int main(int argc, char **argv) {
    char **expr_argv;
    int expr_argc;
    expression_t expression;
    expr_err e_err = EXPR_ERR_NONE;
    find_err f_err = FIND_ERR_NONE;
    int ret = 0;

    if (check_args(argc, argv) < 0) {
        ret = 1;
    }

    else {
        expr_argv = &(argv[2]);
        expr_argc = argc-2;

        e_err = expression_create(&expression, expr_argc, expr_argv);
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
 * Does almost all the work of find. With the expression already parsed this
 *   just needs to create necessary storage and objects for descending the 
 *   tree and output the results once that's done.
 * Returns: FIND_ERR_NONE on success, and FIND_ERR_FTREE or FIND_ERR_MALLOC
 *   otherwise.
 */
find_err find(char *file, expression_t *expression) {
    FTS *file_tree;
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
 *   every file that evaluates to true to path_list. Every call to fts_read
 *   gives the struct of the file back which then becomes the input into
 *   expression_evaluate.
 * Returns: FIND_ERR_NONE on success, and FIND_ERR_MALLOC if a node fails to
 *   be allocated.
 */
find_err descend_tree(FTS *file_tree, expression_t *expression, list *path_list) {
    FTSENT *entry;
    node *n;
    find_err ret = FIND_ERR_NONE;

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
    int ret = 0;
    if (argc == 1) {
        printf("%s: invalid arguments\n", argv[0]);
        printf("Usage: %s file [expression]\n", argv[0]);
        ret = -1;
    }
    return ret;
}

/**
 * Basic error output for expression creation.
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
 * Basic error output for find.
 */
void find_perror(find_err err, char *pname) {
    switch (err) {
    case FIND_ERR_NONE:
        break;
    case FIND_ERR_MALLOC:
        perror(pname);
    case FIND_ERR_FTREE:
        perror(pname);
    }
}