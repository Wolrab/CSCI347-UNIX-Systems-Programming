#include "find.h"

int main(int argc, char **argv) {
    FTS *file_tree;
    char *expr;
    int expr_len ;
    expr_err err = EXPR_ERR_NONE;
    
    if (argc == 1) {
        printf("%s: invalid arguments\n", argv[0]);
        printf("Usage: find [file] [expression]\n");
        return 1;
    }
    if (access(argv[1], F_OK) < 0) {
        printf("%s: file '%s' not found\n", argv[0], argv[1]);
        return 1;
    }

    expr = argv[2];
    expr_len = argc-2;

    err = parse_expr(expr, expr_len);
    switch(err) {
    case EXPR_ERR_NONE:
        break;
    case EXPR_ERR_INVALID_PRIMARY:
        printf("%s: invalid primary '%s' in expression\n", \
                argv[0], expr[0]);
        return 1;
    case EXPR_ERR_INVALID_ARG:
        printf("%s: invalid argument '%s' for primary '%s' in expression\n", \
                argv[0], expr[1], expr[0]);
        return 1;
    }

    file_tree = fts_open(&argv[1], FTS_PHYSICAL, NULL);
    // find

    return 0;
}
