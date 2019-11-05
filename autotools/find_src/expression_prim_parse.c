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
 *   primary_t is an enum, the primary we are looking for is just the index
 *   where primary_str matches str.
 * Returns: 0 on success, and -1 if arg_s does not correspond to a primary.
 */
int primary_parse(primary_t *primary, char *arg_s) {
    int prim = 0;
    while (prim < PRIMARY_NUM && \
            strncmp(arg_s, primary_str[prim], strlen(primary_str[prim])) != 0) {
        prim++;
    }
    *primary = prim;
    return (prim == PRIMARY_NUM ? -1 : 0);
}

/**
 * Starting at arg_i, parses an argument array until either a valid argument for
 *   primary is found or an error occurs.
 * Parsing and moving arg_i is left to helper functions. This function manages
 *   the returns and errors if necessary.
 * Returns: 0 on success, -1 if the arg could not be parsed.
 */
int primary_arg_parse(primary_t primary, primary_arg *arg, \
        char ***arg_i) {
    int ret = 0;
    switch(primary_arg_type[primary]) {
    case ARG_LONG:
        // ret = helperfunction
        break;
    case ARG_CHAR:
        // ret = helperfunction
        break;
    case ARG_CTIM:
        // ret = helperfunction
        break;
    case ARG_ARGV:
        // ret = helperfunction
        break;
    default:
        ret = -1;
    }
    return ret;
}

/**
 * TODO:
 */
long get_arg_long(char ***arg_i) {

}

/**
 * TODO:
 */
char get_arg_char(char ***arg_i) {
    
}

/**
 * Expected Input: A path to a file
 * Consumes: 1 arg
 * Output: A copy of the ctim entry from the file's stat struct or NULL on
 *   error.
 */
struct timespec* get_arg_ctim(char ***arg_i) {
    struct stat f_stat;
    struct timespec *ctim;

    errno = 0;
    ctim = malloc(sizeof(struct timespec));
    if (ctim == NULL) {
        return NULL;
    }

    if (stat((*arg_i)[0], &f_stat) < 0) {
        free(ctim);
        return NULL;
    }

    memcpy(ctim, f_stat.st_ctim, sizeof(struct timespec));
    (*arg_i) = (*arg_i) + 1;

    return ctim;
}

/**
 * Expected Input: An array of args to execute a program terminated by a ';'
 * Consumes: >=2 args, minimum number being the program followed by ';'
 * Output: A pointer to the start of the args formatted appropriately for
 *   execvp(3) or NULL on error.
 */
char** get_arg_argv(char ***arg_i) {
    char **arg_curr = (*arg_i)[0];
    
    int i = 0;
    int arg_end = !strncmp(PRIM_EXEC_ARGS_END, arg_curr[i], \
            strlen(PRIM_EXEC_ARGS_END));
    while (arg_curr[i] != NULL || arg_end) {
        i++;
        arg_end = !strncmp(PRIM_EXEC_ARGS_END, arg_curr[i], \
            strlen(PRIM_EXEC_ARGS_END));
    }

    if (i == 0 || arg_curr[i] == NULL) {
        return NULL;
    }

    arg_curr[i] = NULL;
    (*arg_i) = (*arg_i) + i + 1;
    return arg_curr;
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
