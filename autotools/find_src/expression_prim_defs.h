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
#include <assert.h>

typedef enum primary primary_t;
typedef enum arg_type arg_type;
typedef union primary_arg primary_arg;
typedef struct prog_state prog_state;

// For rounding time to the nearest day/minute
#define SEC_PER_DAY 86400
#define SEC_PER_MIN 60

// For expanding an argument into a path in the EXEC primary
#define PRIM_EXEC_PATH_EXPAND "{}"
// Terminating arg in an argument array for the EXEC primary
#define PRIM_EXEC_ARGV_END ";"

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

// Argument types taken by primaries. 
enum arg_type {
    LONG_ARG = 0,
    CHAR_ARG = 1,
    CTIM_ARG = 2,
    ARGV_ARG = 3
};

// Arrays for mapping any primary to its string representation or argument type
//   respectively.
extern const char *const primary_str_map[];
extern const arg_type primary_arg_type_map[];

// A container holding an argv and the number of arguments.
struct argv_s {
    char **argv;
    int argc;
};

// Holds the arg for any given primary
union primary_arg {
    long long_arg;
    char char_arg;
    struct timespec *ctim_arg;
    struct argv_s *argv_arg;
};

// Holds values representing the program's state that some primaries take as
//   arguments
struct prog_state {
    time_t start_time_day;
    time_t start_time_min;
};

#endif /* __EXPRESSION_PRIM_DEFS_H */