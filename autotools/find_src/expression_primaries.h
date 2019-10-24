#ifndef __EXPRESSION_PRIMARIES
#define __EXPRESSION_PRIMARIES
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

typedef enum primary primary_t;
typedef enum arg_type arg_type;
typedef union primary_arg primary_arg;
typedef struct primary_args_g primary_args_g;

#define SEC_PER_DAY 86400
#define SEC_PER_MIN 60

// Primaries
enum primary {
    CNEWER = 0,
    CMIN  = 1,
    CTIME = 2,
    MMIN  = 3,
    MTIME = 4,
    TYPE  = 5,
    PRIMARY_NUM = 6
};
extern const char *const primary_str[];

// Argument types
enum arg_type {
    ARG_LONG = 0,
    ARG_CHAR = 1,
    ARG_STAT = 2
};
extern const arg_type primary_arg_type[];

union primary_arg {
    long long_arg;
    char char_arg;
    struct stat *stat_arg;
};

// args available to all primaries
struct primary_args_g {
    time_t time_day;
    time_t time_min;
};

// Parsing functions
int primary_parse(primary_t *primary, char *arg_s);
int primary_arg_parse(primary_t primary, primary_arg *arg, char *arg_s);
struct stat* get_stat(char *path);

// Gets state arguments usable by all primaries
int get_primary_globals(primary_args_g *global_args);

// Evaluate a primary against f_stat
bool primary_evaluate(primary_t primary, primary_arg *arg,\
    primary_args_g *globals, struct stat *f_stat);

// Primary evaluation functions
bool eval_cnewer(struct stat *f_stat, struct stat *o_stat);
bool eval_cmin(struct stat *f_stat, long n, primary_args_g *global_args);
bool eval_ctime(struct stat *f_stat, long n, primary_args_g *global_args);
bool eval_mmin(struct stat *f_stat, long n, primary_args_g *global_args);
bool eval_mtime(struct stat *f_stat, long n, primary_args_g *global_args);
bool eval_type(struct stat *f_stat, char t);
char get_type_char(mode_t mode);

#endif /* __EXPRESSION_PRIMARIES */