#ifndef __EXPRESSION_H
#define __EXPRESSION_H
#include <string.h>
#include <errno.h>
#include <math.h>
#include <stdlib.h>
#include "expression_defs.h"

typedef struct primary_node_s primary_node;
typedef struct expression_s expression_t;

struct primary_node_s {
    primary_t primary;
    arg_type type;
    union {
        long long_arg;
        struct stat stat_arg;
        char char_arg;
    } arg;

    primary_node *next;
};

struct time_cmp_s {
    time_t start_time_day;
    time_t start_time_min;
};

struct expression_s {
    struct time_cmp_s time_cmp;
    primary_node *head;
};

// Creates a full expression from an array of argument strings.
// On error, this function will perform swaps between indexes of expr_args for
//   effective error return to the calling function. Do not give this function
//   an array you would like to keep ordered between calls.
expr_err expression_create(expression_t *expression, char **expr_args, \
    int expr_args_size);

// Helper function for handling permutations casewise depending on the error.
void permutate_args(expr_err err, char **expr_args, int i);

// Parse the given args and create and fill a primary node.
expr_err expression_parse_primary_node(char **expr_args, int expr_args_size, \
    int expr_arg_i, primary_node **node);

// Fill the fully allocated primary node
expr_err primary_node_fill(primary_t primary, char *primary_arg_str, \
        primary_node *node);

// Evaluates a full expression object, returning true if all primaries are true
bool expression_evaluate(expression_t *expression, struct stat *f_stat);

// Appends node to expression.
void expression_append(expression_t *expression, primary_node *node);

// Deletes the entire list pointed to by expression.
void expression_delete(expression_t *expression);

// Finds and sets the primary eval func for node
expr_err set_primary_eval_func(primary_t primary, primary_node *node);

// Gets the value of a primary from its raw string representation.
expr_err get_primary(char *expr_element, primary_t *primary);

// Gets the value of a primary argument from its raw string representation.
expr_err get_primary_arg(char *expr_element, primary_node *node);

// Helper function for get_primary_arg
// Handles extraction of a long value from a raw string.
expr_err get_long_arg(long *dest, char *long_arg);

// Helper function for get_primary_arg
// Handles extraction of a stat value from a path.
expr_err get_stat_arg(struct stat **dest, char *path_arg);

// Helper function for get_primary_arg
// Handles extraction of a file_type character from a raw string.
expr_err get_file_type_arg(char *dest, char *file_type_arg);

// Helper function for permuting expr_args on an error.
void permutate_args_invalid_primary(char **expr_args, int primary);

// Helper function for permuting expr_args on an error.
void permutate_args_invalid_primary_arg(char **expr_args, int primary, int primary_arg);

// Swaps indexes i and j in arr.
void swap(char **arr, int i, int j);

// Small utility to make error checking more effective.
bool is_number(char *long_arg);

#endif /* __EXPRESSION_H */