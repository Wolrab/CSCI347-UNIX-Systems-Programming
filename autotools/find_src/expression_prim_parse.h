#ifndef __EXPRESSION_PRIM_PARSE_H
#define __EXPRESSION_PRIM_PARSE_H
#include "expression_prim_defs.h"

// Terminating arg in an argument array for the EXEC primary
#define PRIM_EXEC_ARGS_END ";"

// Parses arg_s and stores its equivalent primary_t in primary.
int primary_parse(primary_t *primary, char *arg_s);

// Parses arg_i as the starting element in an array of args for primary.
//   Consumes args until a valid argument for primary is found or an error
//   occurs. Result is stored in arg.
// After successfull parsing, arg_i points to the element in the arg array
//   immediately after the last consumed element, so the arg array must be null-
//   terminated.
int primary_arg_parse(primary_t primary, primary_arg *arg, char ***arg_i);

// Helpers for primary_arg_parse
long get_arg_long(char ***arg_i);
char get_arg_char(char ***arg_i);
struct timespec* get_arg_ctim(char ***arg_i);
char** get_arg_argv(char ***arg_i);

// Gets arguments usable by all primaries from the program/system state
//   and stores them in global_args.
int get_primary_globals(primary_args_g *global_args);

#endif /* __EXPRESSION_PRIM_PARSE_H */