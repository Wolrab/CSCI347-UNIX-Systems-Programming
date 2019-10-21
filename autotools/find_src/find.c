#include "find.h"

int main(int argc, char **argv) {
    char **expr_args;
    int expr_args_size;
    expression_t expression;

    expr_err err = EXPR_ERR_NONE;
    int ret = 0;

    struct timespec tm;
    
    /*
    if (argc == 1) {
        printf("%s: invalid arguments\n", argv[0]);
        printf("Usage: find [file] [expression]\n");
        return 1;
    }
    if (access(argv[1], F_OK) < 0) {
        printf("%s: file '%s' not found\n", argv[0], argv[1]);
        return 1;
    }

    expr_args = argv[2];
    expr_args_size = argc-2;*/

    expr_args = &(argv[1]);
    expr_args_size = argc-1;

    err = create_expression(&expression, expr_args, expr_args_size);
    if (err != EXPR_ERR_NONE) {
        expression_perror(err, argv, expr_args);
        return 1;
    }

    clock_gettime(CLOCK_REALTIME, &tm);
    set_start_time(tm.tv_sec);

    //ret = find(&(argv[1]), &expression);

    return ret;
}

int find(char **file, expression_t *expression) {
    FTS *file_tree;
    file_tree = fts_open(file, FTS_PHYSICAL, NULL);
}

void expression_perror(expr_err err, char **argv, char **expr_args) {
    switch(err) {
    case EXPR_ERR_MALLOC:
        perror(argv[0]);
        break;
    case EXPR_ERR_INVALID_PRIMARY:
        fprintf(stderr, "%s: invalid primary '%s' in expression\n", \
            argv[0], expr_args[0]);
        break;
    case EXPR_ERR_INVALID_ARG:
        fprintf(stderr, "%s: invalid argument '%s' for primary '%s' in "
            "expression\n", argv[0], expr_args[1], expr_args[0]);
        break;
    case EXPR_ERR_NO_ARG:
        fprintf(stderr, "%s: no argument found for primary '%s'", \
            argv[0], expr_args[0]);
        break;
    }
}