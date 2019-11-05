#ifndef __EXPRESSION_PRIM_H
#define __EXPRESSION_PRIM_H
#include <unistd.h>
#include <sys/wait.h>
#include "expression_prim_defs.h"

#define PRIM_EXEC_PATH_EXPAND "{}"

// Evaluates a primary against entry.
bool primary_evaluate(primary_t primary, primary_arg *arg,\
    primary_args_g *globals, FTSENT *entry);

// Primary evaluation functions
bool eval_cnewer(struct timespec *ctim, struct timespec *o_ctim);
bool eval_cmin(struct timespec *ctim, long n, primary_args_g *global_args);
bool eval_ctime(struct timespec *ctim, long n, primary_args_g *global_args);
bool eval_mmin(struct timespec *mtim, long n, primary_args_g *global_args);
bool eval_mtime(struct timespec *mtim, long n, primary_args_g *global_args);
bool eval_type(mode_t mode, char t);
bool eval_exec(char *path, argv_t argv);

// Helpers for primary evaluation functions
char get_type_char(mode_t mode);

#endif /* __EXPRESSION_PRIM_H */