#include "find.h"

int main(int argc, char **argv) {
    FTS *file_tree;
    char *expression;
    int expression_len;
    
    if (argc == 1) {
        printf("%s: invalid arguments\n", argv[0]);
        printf("Usage: find [file] [expression]\n");
        return 1;
    }
    if (access(argv[1], F_OK) < 0) {
        printf("%s: file '%s' not found\n", argv[0], argv[1]);
        return 1;
    }

    expression = argv[2];
    expression_len = argc-2;
    switch(validate_expression(expression, expression_len)) {
    case EXPR_ERR_NONE:
        break;
    case EXPR_ERR_INVALID_PRIMARY:
        printf("%s: invalid primary '%s' in expression\n", \
                argv[0], expression[0]);
        return 1;
    case EXPR_ERR_INVALID_ARG:
        printf("%s: invalid argument '%s' for primary '%s' in expression\n", \
                argv[0], expression[1], expression[0]);
        return 1;
    }

    file_tree = fts_open(&argv[1], FTS_PHYSICAL, NULL);
    // find

    return 0;
}

// Checks if an expression is valid
expression_error validate_expression(char **expression, int expression_len) {

}