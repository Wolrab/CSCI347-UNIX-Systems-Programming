#ifndef __EXPRESSION_PRIM_EVAL_H
#define __EXPRESSION_PRIM_EVAL_H
#include <unistd.h>
#include <sys/wait.h>
#include "expression_prim_defs.h"

// Evaluates a primary against entry.
bool primary_evaluate(primary_t primary, primary_arg *arg,\
    prog_state *state_args, FTSENT *entry);

// Primary evaluator functions
bool eval_cnewer(struct timespec *ctim, struct timespec *o_ctim);
bool eval_cmin(struct timespec *ctim, long n, time_t start_time_min);
bool eval_ctime(struct timespec *ctim, long n, time_t start_time_day);
bool eval_mmin(struct timespec *mtim, long n, time_t start_time_min);
bool eval_mtime(struct timespec *mtim, long n, time_t start_time_day);
bool eval_type(mode_t mode, char t);
bool eval_exec(char *path, char **argv, char **argv_dest, int argc);

// Helper for primary evaluator functions
char get_type_char(mode_t mode);

#endif /* __EXPRESSION_PRIM_EVAL_H */