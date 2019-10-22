#ifndef __EXPRESSION_H
#define __EXPRESSION_H
#include <string.h>
#include <errno.h>
#include <math.h>
#include <stdlib.h>
#include "expression_defs.h"

// primary_arg_* structs are all wrappers for the different arguments a 
//   primary's evaluation function can take. They include a field for the 
//   argument itself and a pointer to the primary's evaluation function.

// long argument wrapper
struct primary_arg_long {
    long arg;
    bool (*eval)(struct stat*, long);
};

// stat argument wrapper
struct primary_arg_stat {
    struct stat *arg;
    bool (*eval)(struct stat*, struct stat*);
};

// file type argument wrapper
struct primary_arg_file_type {
    char arg;
    bool (*eval)(struct stat*, char);
};

typedef struct primary_node primary_node;
typedef primary_node* expression_t;

// A generic primary_node. The type field determines the type of struct 
//   that primary_arg_type represents.
struct primary_node {
    arg_type type;
    primary_node *next;
    union {
        struct primary_arg_long long_t;
        struct primary_arg_stat stat_t;
        struct primary_arg_file_type file_type_t;
    } primary_arg_type;
};

// Creates a full expression from an array of argument strings.
// Do not pass this function any expression pointer value that points to any
//   initialized data. It will be overwritten and lost.
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