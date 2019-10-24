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

// Iterates through the file tree originating from file and evaluating paths 
//   against the expression.
int find(char **file, expression_t *expression, list *path_list);

// Simple output to stdout of the list of paths.
void output_path_list(list *path_list);

// Error output for errors returned from expression_create
void expression_perror(expr_err err, char **argv, char **expr_args);

#endif /* __FIND_H */