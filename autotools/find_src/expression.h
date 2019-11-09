#ifndef __EXPRESSION_H
#define __EXPRESSION_H
#include <string.h>
#include <errno.h>
#include <math.h>
#include <stdlib.h>
#include <assert.h>
#include "expression_prim_eval.h"
#include "expression_prim_parse.h"

typedef struct primary_node primary_node;
typedef struct expression expression_t;
typedef enum expr_err expr_err;

// A node representing one primary.
struct primary_node {
    primary_t primary;
    primary_arg arg;

    primary_node *next;
};

// expression struct which includes state information about the program.
struct expression {
    prog_state state_args;
    primary_node *head;
};

// Error defines
enum expr_err {
    EXPR_ERR_NONE    = 0,
    EXPR_ERR_MALLOC  = 1,
    EXPR_ERR_GLOBALS = 2,
    EXPR_ERR_PRIMARY = 3,
    EXPR_ERR_ARG     = 4,
    EXPR_ERR_NO_ARG  = 5
};

// Creates an expression. expression is expected to be already allocated.
//   expr_argv must be null-terminated.
expr_err expression_create(expression_t *expression, char **expr_argv);

// Creates a primary node. Allocation of primary_node is done here, so node must
//   point to valid memory. primary_arg_i is moved to the next index after most
//   recently parsed arg and must be NULL terminated.
expr_err expression_create_primary(primary_node **node, char *primary_str, \
    char ***primary_arg_i);

// Adds a primary node to the expression.
void expression_add_primary(expression_t *expression, primary_node *node);

// Evaluates the expression against entry.
bool expression_evaluate(expression_t *expression, FTSENT *entry);

// Deletes the expression.
void expression_delete(expression_t *expression);
#endif /* __EXPRESSION_H */