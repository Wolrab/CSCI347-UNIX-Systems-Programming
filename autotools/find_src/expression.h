#ifndef __EXPRESSION_H
#define __EXPRESSION_H
#include <string.h>
#include <errno.h>
#include <math.h>
#include <stdlib.h>
#include "expression_primaries.h"

typedef struct primary_node primary_node;
typedef struct expression expression_t;
typedef enum expr_err expr_err;

struct primary_node {
    primary_t primary;
    primary_arg arg;

    primary_node *next;
};

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

expr_err expression_create(expression_t *expression, int expr_argc, \
    char **expr_argv);

expr_err expression_create_primary(primary_node **node, char *primary_s, \
    char *primary_arg_s);

void expression_add_primary(expression_t *expression, primary_node *node);

bool expression_evaluate(expression_t *expression, struct stat *f_stat);

void expression_delete(expression_t *expression);
#endif /* __EXPRESSION_H */