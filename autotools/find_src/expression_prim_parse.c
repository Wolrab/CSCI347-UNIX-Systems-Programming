/**
 * TODO:
 */
#include "expression_prim_parse.h"

// Global definitions of mappings for primary_t enums to their corresponding
//   string representations and arg types
const char *const primary_str[] = {"-cnewer", "-cmin", "-ctime", "-mmin", \
    "-mtime", "-type", "-exec"};
const arg_type primary_arg_type[] = {ARG_CTIM, ARG_LONG, ARG_LONG, ARG_LONG, \
    ARG_LONG, ARG_CHAR, ARG_ARGV};

/**
 * Parses arg_s and puts the corresponding primary_t into primary. Since
 *   primary_t is an enum, the primary we are looking for is the index
 *   where primary_str matches str.
 * Returns: 0 on success, and -1 if arg_s does not correspond to a primary.
 */
int primary_parse(primary_t *primary, char *arg_s) {
    unsigned int prim = 0;
    int ret = 0;
    while (prim < PRIMARY_NUM && \
            strncmp(arg_s, primary_str[prim], strlen(primary_str[prim])) != 0) {
        prim++;
    }
    if (prim == PRIMARY_NUM) {
        ret = -1;
    }
    else {
        *primary = prim;
    }
    return ret;
}

/**
 * Starting at arg_i, parses an argument array until either a valid argument for
 *   primary is found or an error occurs.
 * Parsing and moving arg_i is left to helper functions. This function manages
 *   the returns and errors if necessary.
 * Returns: 0 on success, -1 if the arg could not be parsed.
 */
int primary_arg_parse(primary_t primary, primary_arg *arg, argv_t *arg_i) {
    int ret = 0;
    switch(primary_arg_type[primary]) {
    case ARG_LONG:
        ret = get_arg_long(arg, arg_i);
        break;
    case ARG_CHAR:
        ret = get_arg_char(arg, arg_i);
        break;
    case ARG_CTIM:
        ret = get_arg_ctim(arg, arg_i);
        break;
    case ARG_ARGV:
        ret = get_arg_argv(arg, arg_i);
        break;
    default:
        ret = -1;
    }
    return ret;
}

/**
 * Expected arg: A string representing a long integer
 * Consumes: 1 arg
 * Returns: 0 on success or -1 on error
 */
int get_arg_long(primary_arg *arg, argv_t *arg_i) {
    arg->long_arg = strtol((*arg_i)[0], NULL, 10);
    incr_arg(arg_i, 1);
    return 0;
}

/**
 * Expected arg: A single character
 * Consumes: 1 arg
 * Returns: 0 on success or -1 on error
 */
int get_arg_char(primary_arg *arg, argv_t *arg_i) {
    int ret = 0;
    if (strlen((*arg_i)[0]) != 1) {
        ret = -1;
    }
    else {
        arg->char_arg = (*arg_i)[0][0];
        incr_arg(arg_i, 1);
    }
    return ret;
}

/**
 * Expected arg: A path to a file
 * Consumes: 1 arg
 * Returns: 0 on success or -1 on error
 */
int get_arg_ctim(primary_arg *arg, argv_t *arg_i) {
    struct stat f_stat;
    struct timespec *ctim;
    int ret = 0;

    errno = 0;
    ctim = malloc(sizeof(struct timespec));
    if (ctim == NULL) {
        ret = -1;
    }
    else if (stat((*arg_i)[0], &f_stat) < 0) {
        free(ctim);
        ret = -1;
    }
    else {
        memcpy(ctim, &(f_stat.st_ctim), sizeof(struct timespec));
        arg->ctim_arg = ctim;
        incr_arg(arg_i, 1);
    }
    return ret;
}

/**
 * Expected args: An array of args to execute a program terminated by
 *   PRIM_EXEC_ARGS_END.
 * Consumes: >=2 args, minimum number being the program followed by
 *   PRIM_EXEC_ARGS_END.
 * Returns: 0 on success or -1 if the args are not terminated with
 *   PRIM_EXEC_ARGS_END.
 */
#include <stdio.h>
int get_arg_argv(primary_arg *arg, argv_t *arg_i) {
    int ret = 0, i = 0;
    argv_t argv_cpy;

    while ((*arg_i)[i] != NULL && strncmp(PRIM_EXEC_ARGS_END, (*arg_i)[i], \
            strlen((*arg_i)[i]) + 1)) {
        i++;
    }

    if ((*arg_i)[i] == NULL) {
        ret = -1;
    }
    else {
        errno = 0;
        argv_cpy = malloc(sizeof(char*) * (i + 1));
        if (argv_cpy == NULL) {
            ret = -1;
        }
        else {
            memcpy(&argv_cpy, arg_i, i * sizeof(char*));
            argv_cpy[i] = NULL;
            arg->argv_arg = argv_cpy;
            incr_arg(arg_i, i + 1);
        }
    }
    return ret;
}

void incr_arg(argv_t *arg_i, int i) {
    *arg_i = (*arg_i) + i;
}

/**
 * Fills out global_args with necessary program and state information.
 * Currently the only values in use are the number of minutes since the epoch
 *   and the number of days since the epoch, rounded up. The resolution of this
 *   calculation is only in seconds, there is no need to go into the
 *   nanoseconds.
 * Also, the name is misleading. The args are global in the sense that their
 *   values are the same between different primaries, and all primaries have
 *   access to them if needed.
 * Returns: 0 on success, and -1 on error.
 */
int get_primary_globals(primary_args_g *global_args) {
    struct timespec tm;
    if(clock_gettime(CLOCK_REALTIME, &tm) < 0) {
        return -1;
    }

    global_args->time_day = tm.tv_sec / SEC_PER_DAY;
    if (tm.tv_sec % SEC_PER_DAY > 0) {
        global_args->time_day++;
    }
    
    global_args->time_min = tm.tv_sec / SEC_PER_MIN;
    if (tm.tv_sec % SEC_PER_MIN > 0) {
        global_args->time_min++;
    }

    return 0;
}
