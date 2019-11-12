/**
 * Interface for parsing primaries and their arguments. primary_parse converts
 *   a given user string into its equivalent primary_t value, which is then used
 *   for all subsequent primary parsing and evaluating. get_prog_state is also
 *   included here as its implementation is related to the types of primaries
 *   and their arguments.
 */
#include "expression_prim_parse.h"

// Arrays representing mappings from primary_t enums to their string
//   representations and arg types respectively.
const char *const primary_str_map[] = {"-cnewer", "-cmin", "-ctime", "-mmin", \
    "-mtime", "-type", "-exec"};
const arg_type primary_arg_type_map[] = {CTIM_ARG, LONG_ARG, LONG_ARG, LONG_ARG, \
    LONG_ARG, CHAR_ARG, ARGV_ARG};

/**
 * Parses primary_str_map and puts the corresponding primary_t into primary.
 *   Since primary_t is an enum, the primary we are looking for is the index
 *   where primary_str_map matches primary_str.
 * Returns 0 on success, and -1 if primary_str does not correspond to a
 *   primary.
 */
int primary_parse(primary_t *primary, char *primary_str) {
    unsigned int prim = 0;
    int ret = 0;
    while (prim < PRIMARY_NUM && strncmp(primary_str, primary_str_map[prim], \
            strlen(primary_str_map[prim])) != 0) {
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
 * Starting at argv_i, parses an argument array until either a valid argument
 *   for primary is found or an error occurs. On success, argv_i points to the
 *   value immediately after the last consumed argument.
 * Parsing and moving argv_i is left to the appropriate get_arg_* function. This
 *   function simply manages the returns and errors if necessary.
 * Returns 0 on success, -1 if the arg could not be parsed or doesn't exist.
 */
int primary_arg_parse(primary_t primary, primary_arg *arg, char ***argv_i) {
    int ret = 0;
    switch(primary_arg_type_map[primary]) {
    case LONG_ARG:
        ret = get_arg_long(arg, argv_i);
        break;
    case CHAR_ARG:
        ret = get_arg_char(arg, argv_i);
        break;
    case CTIM_ARG:
        ret = get_arg_ctim(arg, argv_i);
        break;
    case ARGV_ARG:
        ret = get_arg_argv(arg, argv_i);
        break;
    default:
        ret = -1;
    }
    return ret;
}

/**
 * Expected argv value: A string representing a long integer
 * Consumes: 1 arg
 * Returns 0 on success and -1 if argv_i could not fully be converted to an
 *   integer.
 */
int get_arg_long(primary_arg *arg, char ***argv_i) {
    char *end_ptr;
    long val;
    int ret = 0;

    val = strtol((*argv_i)[0], &end_ptr, 10);
    if (end_ptr != NULL) {
        ret = -1;
    }
    else {
        arg->long_arg = val;
        incr_argv_i(argv_i, 1);
    }
    return 0;
}

/**
 * Expected argv value: A single character
 * Consumes: 1 arg
 * Returns 0 on success or -1 if the argument is more than one character.
 */
int get_arg_char(primary_arg *arg, char ***argv_i) {
    int ret = 0;
    if (strlen((*argv_i)[0]) != 1) {
        ret = -1;
    }
    else {
        arg->char_arg = (*argv_i)[0][0];
        incr_argv_i(argv_i, 1);
    }
    return ret;
}

/**
 * Expected argv value: A path to a file
 * Consumes: 1 arg
 * Returns 0 on success or -1 on error
 */
int get_arg_ctim(primary_arg *arg, char ***argv_i) {
    struct stat f_stat;
    struct timespec *ctim;
    int ret = 0;

    errno = 0;
    ctim = malloc(sizeof(struct timespec));
    if (ctim == NULL) {
        ret = -1;
    }
    else if (stat((*argv_i)[0], &f_stat) < 0) {
        free(ctim);
        ret = -1;
    }
    else {
        memcpy(ctim, &(f_stat.st_ctim), sizeof(struct timespec));
        arg->ctim_arg = ctim;
        incr_argv_i(argv_i, 1);
    }
    return ret;
}

/**
 * Expected argv values: An array of args to execute a program terminated by
 *   PRIM_EXEC_ARGV_END.
 * Consumes: >=2 args, minimum number being the program followed by
 *   PRIM_EXEC_ARGV_END.
 * Returns 0 on success, -1 if an error occured or if the args are not
 *   terminated with PRIM_EXEC_ARGV_END.
 */
int get_arg_argv(primary_arg *arg, char ***argv_i) {
    int ret = 0;
    struct argv_s *argv_s;

    int argc = 0;
    while ((*argv_i)[argc] != NULL && strncmp(PRIM_EXEC_ARGV_END, \
            (*argv_i)[argc], strlen(PRIM_EXEC_ARGV_END) + 1)) {
        argc++;
    }
    if (argc == 0 || (*argv_i)[argc] == NULL) {
        ret = -1;
    }
    else {
        errno = 0;
        argv_s = malloc(sizeof(struct argv_s));
        if (argv_s == NULL) {
            ret = -1;
        }
        else {
            errno = 0;
            argv_s->argv = malloc(sizeof(void*) * (argc + 1));
            if (argv_s->argv == NULL) {
                free(argv_s);
                ret = -1;
            }
            else {
                errno = 0;
                argv_s->argv_dest = malloc(sizeof(void*) * (argc + 1));
                if (argv_s->argv_dest == NULL) {
                    free(argv_s->argv);
                    free(argv_s);
                }
                else {
                    memcpy(argv_s->argv, *argv_i, argc * sizeof(void*));
                    memset(argv_s->argv_dest, '\0', argc * sizeof(void*));
                    argv_s->argv[argc] = NULL;
                    argv_s->argc = argc;

                    arg->argv_arg = argv_s;
                    incr_argv_i(argv_i, argc + 1);
                }
            }
        }
    }
    return ret;
}

/**
 * Increments the value pointed at by argv_i by i
 */
void incr_argv_i(char ***argv_i, int i) {
    *argv_i = (*argv_i) + i;
}

/**
 * Deletes arg given the primary it corresponds to. The specific implementation
 *   of each get_arg_* function determines the behavior of this function.
 */
void primary_delete_arg(primary_t primary, primary_arg *arg) {
    switch(primary_arg_type_map[primary]) {
    case LONG_ARG:
        break;
    case CHAR_ARG:
        break;
    case CTIM_ARG:
        free(arg->ctim_arg);
        break;
    case ARGV_ARG:
        free(arg->argv_arg->argv);
        free(arg->argv_arg);
        break;
    }
}

/**
 * Fills out state_args with information from the program's state.
 * Currently the only values in use are the number of minutes since the epoch
 *   and the number of days since the epoch, rounded up. The resolution of this
 *   calculation is only in seconds, nanosecond resolution would be very
 *   overkill given the specific use of these values.
 * Returns 0 on success, and -1 on error.
 */
int get_prog_state(prog_state *state_args) {
    int ret = 0;
    struct timespec tm;
    if (clock_gettime(CLOCK_REALTIME, &tm) < 0) {
        ret = -1;
    }
    else {
        state_args->start_time_day = tm.tv_sec / SEC_PER_DAY;
        if (tm.tv_sec % SEC_PER_DAY > 0) {
            state_args->start_time_day++;
        }

        state_args->start_time_min = tm.tv_sec / SEC_PER_MIN;
        if (tm.tv_sec % SEC_PER_MIN > 0) {
            state_args->start_time_min++;
        }
    }
    return ret;
}
