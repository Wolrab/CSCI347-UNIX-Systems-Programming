//TODO: Short blurb

#ifndef __EXPRESSION_PRIM_DEFS_H
#define __EXPRESSION_PRIM_DEFS_H
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <fts.h>

typedef enum primary primary_t;
typedef enum arg_type arg_type;
typedef union primary_arg primary_arg;
typedef struct primary_args_g primary_args_g;

// For finding the nearest day
#define SEC_PER_DAY 86400
#define SEC_PER_MIN 60

// For expanding an argument into a path in the EXEC primary
#define PRIM_EXEC_PATH_EXPAND "{}"

// Primaries
enum primary {
    CNEWER = 0,
    CMIN   = 1,
    CTIME  = 2,
    MMIN   = 3,
    MTIME  = 4,
    TYPE   = 5,
    EXEC   = 6,
    PRIMARY_NUM = 7
};

// Argument types
enum arg_type {
    ARG_LONG = 0,
    ARG_CHAR = 1,
    ARG_CTIM = 2,
    ARG_ARGV = 3
};

// Arrays for mapping any primary to its string representation or argument type
//   respectively. Defined in expression_prim_parse.h
extern const char *const primary_str[];
extern const arg_type primary_arg_type[];

// Holds the arg for any given primary
union primary_arg {
    long long_arg;
    char char_arg;
    struct timespec *ctim_arg;
    char **argv_arg;
};

// Args available to all primaries
struct primary_args_g {
    time_t time_day;
    time_t time_min;
};

#endif /* __EXPRESSION_PRIM_DEFS_H */