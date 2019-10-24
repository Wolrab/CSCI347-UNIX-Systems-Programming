#ifndef __FIND_H
#define __FIND_H
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fts.h>
#include <stdio.h>
#include <time.h>
#include "list.h"
#include "expression.h"

typedef enum find_err find_err;

enum find_err {
    FIND_ERR_NONE = 0,
    FIND_ERR_MALLOC = 1,
    FIND_ERR_FTREE = 2
};

// Iterates through the file tree originating from file and evaluating paths 
//   against the expression.
find_err find(char **file, expression_t *expression);

// Decends the tree, adding to path_list given the evaluation of expression
//   for each file.
find_err descend_tree(FTS *file_tree, expression_t *expression, list *path_list);

// Simple output to stdout of the list of paths.
void output_path_list(list *path_list);

// Simple argument validator/checker.
int check_args(int argc, char **argv);

// Error output for errors returned from expression_create.
void expression_perror(expr_err err, char *pname);

#endif /* __FIND_H */