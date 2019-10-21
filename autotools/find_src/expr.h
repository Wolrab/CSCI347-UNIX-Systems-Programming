#ifndef __FIND_EXPR
#define __FIND_EXPR

typedef enum arg_type arg_type;
enum arg_type {
    INT = 0,
    CHAR = 1,
    FILE = 2
};

typedef enum expr_primaries;
enum expr_primaries {
    CMIN = 0,
    CNEWER = 1,
    CTIME = 2,
    CMIN = 3,
    MTIME = 4,
    TYPE = 5
};

extern const char *const expr_primaries_str[];
extern const arg_type expr_primaries_arg_type[];

typedef enum expr_operators expr_operators;
enum expr_operators {
    AND = 0
};
extern char *const expr_operators_str[] = {"a"};

// Errors for 
typedef enum expr_err expr_err;
enum expr_err {
    EXPR_ERR_NONE = 0,
    EXPR_ERR_INVALID_PRIMARY = 1,
    EXPR_ERR_INVALID_ARG = 2
};

#endif /* __FIND_EXPR */