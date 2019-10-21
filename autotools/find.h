#ifndef __FIND_H
#define __FIND_H
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fts.h>
#include <stdio.h>
#include <time.h>

// Iterates through the file tree
int find(FTS *file_tree, char **expression, int expression_len);

const char *const cmin = "cmin";
const char *const cnewer = "cnewer";
const char *const ctime = "ctime";
const char *const cmin = "cmin";
const char *const cmin = "cmin";

typedef enum expression_error {
    EXPR_ERR_NONE = 0,
    EXPR_ERR_INVALID_PRIMARY = 1,
    EXPR_ERR_INVALID_ARG = 2
} expression_error;

// Checks if an expression is valid
expression_error validate_expression(char **expression, int expression_len);
// Evaluates a given expression
bool evaluate_expression(char **expression, int expression_len);

// Expression primaries
bool prim_cmin(FTSENT *ent, int n);
bool prim_cnewer(FTSENT *ent, char *file);
bool prim_ctime(FTSENT *ent, int n);
bool prim_mmin(FTSENT *ent, int n);
bool prim_mtime(FTSENT *ent, int n);
bool prim_type(FTSENT *ent, char t);

#endif /* __FIND_H */