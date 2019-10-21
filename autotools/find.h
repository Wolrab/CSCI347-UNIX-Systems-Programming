#ifndef __FIND_H
#define __FIND_H
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fts.h>
#include <stdio.h>
#include <time.h>
#include "find_list.h"
#include "find_expr.h"

// Iterates through the file tree
int find(FTS *file_tree, char **expression, int expression_len);

// Checks if an expression is valid
expr_err validate_expression(char **expression, int expression_len);
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