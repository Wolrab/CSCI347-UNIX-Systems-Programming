#include "find.h"

/**
 * 
 */
int main(int argc, char **argv) {
    char **expr_args;
    int expr_args_size;
    expression_t expression;

    expr_err err = EXPR_ERR_NONE;
    int ret = 0;

    if (argc == 1) {
        printf("%s: invalid arguments\n", argv[0]);
        printf("Usage: %s file [expression]\n", argv[0]);
        return 1;
    }

    if (access(argv[1], F_OK) < 0) {
        printf("%s: file '%s' not found\n", argv[0], argv[1]);
        return 1;
    }

    expr_args = &(argv[2]);
    expr_args_size = argc-2;

    err = expression_create(&expression, expr_args, expr_args_size);
    if (err != EXPR_ERR_NONE) {
        expression_perror(err, argv, expr_args);
        return 1;
    }

    ret = init_global_expression_states();
    if (ret) {
        perror(argv[0]);
        return ret;
    }

    //ret = find(&(argv[1]), &expression);
    if (ret) {
        // do things
    }

    return ret;
}

int init_global_expression_states() {
    struct timespec tm;
    int ret = 0;

    errno = 0;
    ret = clock_gettime(CLOCK_REALTIME, &tm);
    if (ret) {
        return ret;
    }
    set_start_time(tm.tv_sec);
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