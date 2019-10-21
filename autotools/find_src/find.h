#ifndef __FIND_H
#define __FIND_H
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fts.h>
#include <stdio.h>
#include <time.h>
#include "path_list.h"
#include "expression_defs.h"
#include "expression.h"

// Iterates through the file tree
int find(char **file, expression_t *expression);
void expression_perror(expr_err err, char **argv, char **expr_args);

#endif /* __FIND_H */