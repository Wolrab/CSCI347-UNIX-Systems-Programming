#ifndef __EXPRESSION_H
#define __EXPRESSION_H
#include <string.h>
#include <errno.h>
#include <math.h>
#include <stdlib.h>
#include <assert.h>
#include "expression_primaries.h"

typedef struct primary_node primary_node;
typedef struct expression expression_t;
typedef enum expr_err expr_err;

// A node representing one primary.
struct primary_node {
    primary_t primary;
    primary_arg arg;

    primary_node *next;
};

// expression struct which includes global primary agruments
struct expression {
    primary_args_g global_args;
    primary_node *head;
};

// Error defines
enum expr_err {
    EXPR_ERR_NONE = 0,
    EXPR_ERR_MALLOC = 1,
    EXPR_ERR_GLOBALS = 2,
    EXPR_ERR_PRIMARY = 3,
    EXPR_ERR_ARG = 4,
    EXPR_ERR_NO_ARG = 5
};

// Creates an expression. expression is expected to be already allocated.
expr_err expression_create(expression_t *expression, int expr_argc, \
    char **expr_argv);

// Creates a primary node. Allocation of primary_node is done here, the
//   primary_node pointer that node points too must exist though.
expr_err expression_create_primary(primary_node **node, char *primary_s, \
    char *primary_arg_s);

// Adds a primary node to expression.
void expression_add_primary(expression_t *expression, primary_node *node);

// Evaluates the given expression against f_stat
bool expression_evaluate(expression_t *expression, struct stat *f_stat);

// Deletes expression.
void expression_delete(expression_t *expression);
#endif /* __EXPRESSION_H */