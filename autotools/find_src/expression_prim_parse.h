#ifndef __EXPRESSION_PRIM_PARSE_H
#define __EXPRESSION_PRIM_PARSE_H
#include "expression_prim_defs.h"

// Parses arg_s and stores its equivalent primary_t in primary.
int primary_parse(primary_t *primary, char *primary_str);

// Consumes args starting at the element pointed to by argv_i until a valid
//   argument for primary is found or an error occurs. The type of parsing done
//   is determined by primary given.
// argv_i is permutated by this function and must be NULL terminated. Result of
//   the parsing is stored in arg, so it must already be allocated.
int primary_arg_parse(primary_t primary, primary_arg *arg, char ***argv_i);

// Helpers for primary_arg_parse
int get_arg_long(primary_arg *arg, char ***argv_i);
int get_arg_char(primary_arg *arg, char ***argv_i);
int get_arg_ctim(primary_arg *arg, char ***argv_i);
int get_arg_argv(primary_arg *arg, char ***arg_i);

// Increments the value pointed at by argv_i by i
void incr_argv_i(char ***argv_i, int i);

// Deletes arg given the primary it corresponds to.
void primary_delete_arg(primary_t primary, primary_arg *arg);

// Gets arguments usable by all primaries from the program's state and stores
//   them in state_args.
int get_prog_state(prog_state *state_args);

#endif /* __EXPRESSION_PRIM_PARSE_H */