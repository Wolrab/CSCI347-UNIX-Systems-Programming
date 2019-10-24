#ifndef __EXPRESSION_DEFS_H
#define __EXPRESSION_DEFS_H
#include <sys/stat.h>
#include <sys/types.h>
#include <stdbool.h>
#include <time.h>

// Argument types
typedef enum arg_type arg_type;
enum arg_type {
    ARG_LONG = 0,
    ARG_FILE_TYPE = 1,
    ARG_STAT = 2
};

extern const char *const valid_file_types;

// Primaries available
typedef enum primary primary_t;
enum primary {
    CNEWER = 0,
    CMIN  = 1,
    CTIME = 2,
    MMIN  = 3,
    MTIME = 4,
    TYPE  = 5,
    PRIMARIES_TOTAL = 6
};
extern const char *const primary_str_arr[];
extern const arg_type primary_arg_type_arr[];

#define SEC_PER_DAY 86400
#define SEC_PER_MIN 60

// Errors for parsing expressions
typedef enum expr_err expr_err;
enum expr_err {
    EXPR_ERR_NONE = 0,
    EXPR_ERR_MALLOC = 1,
    EXPR_ERR_INVALID_PRIMARY = 2,
    EXPR_ERR_INVALID_ARG = 3,
    EXPR_ERR_NO_ARG = 4
};

// Initialize start_time variables. If this function is not called, the 
//   behavior of many primary evaluator functions is undefined
void set_start_time(time_t start);

// Primary evaluator functions
bool eval_cnewer(struct stat *f_stat, struct stat *o_stat);
bool eval_cmin(struct stat *f_stat, long n);
bool eval_ctime(struct stat *f_stat, long n);
bool eval_mmin(struct stat *f_stat, long n);
bool eval_mtime(struct stat *f_stat, long n);
bool eval_type(struct stat *f_stat, char t);
char get_type_char(mode_t mode);

#endif /* __EXPRESSION_DEFS_H */