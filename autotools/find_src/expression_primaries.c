/**
 * Defines the functionality for parsing and evaluating primaries. Keeps the
 *   implementation details hidden from expression to reduce program complexity.
 * Two arrays, primary_str and primary_arg_type encode all the information
 *   needed for parsing a primary. primary_str maps from user-provided args to
 *   a primary_t, and primary_arg_type maps from a primary_t to the data type
 *   that argument accepts.
 */
#include "expression_primaries.h"

// Array definitions for parsing primaries
const char *const primary_str[] = {"-cnewer", "-cmin", "-ctime", "-mmin", \
    "-mtime", "-type"};
const arg_type primary_arg_type[] = {ARG_STAT, ARG_LONG, ARG_LONG, ARG_LONG, \
    ARG_LONG, ARG_CHAR};

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
 * Parses arg_s for its value as an arg for the given primary, putting its
 *   value in arg.
 * Returns: 0 on success, -1 if the arg could not be parsed.
 */
int primary_arg_parse(primary_t primary, primary_arg *arg, char *arg_s) {
    int ret = 0;
    switch(primary_arg_type[primary]) {
    case ARG_LONG:
        arg->long_arg = strtol(arg_s, NULL, 10);
        break;
    case ARG_CHAR:
        arg->char_arg = arg_s[0];
        if (strlen(arg_s) != 1) {
            ret = -1;
        }
        break;
    case ARG_STAT:
        arg->stat_arg = get_stat(arg_s);
        if (arg->stat_arg == NULL) {
            ret = -1;
        }
        break;
    default:
        ret = -1;
    }
    return ret;
}

/**
 * Helper function that stat's the file at path.
 * Returns: stat structure if sucessfull, NULL otherwise.
 */
struct stat* get_stat(char *path) {
    struct stat *f_stat;
    
    errno = 0;
    f_stat = malloc(sizeof(struct stat));
    if (f_stat == NULL) {
        return NULL;
    }
    if (stat(path, f_stat) < 0) {
        free(f_stat);
        f_stat = NULL;
    }
    return f_stat;
}

/**
 * Fills out global_args with necessary program and state information.
 * The name is misleading, the args are global in the sense that they do not
 *   have different values between different primaries, and all primaries have
 *   access to them if they need it.
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

/**
 * Takes a primary, its arg value, and globals, and returns its truth value
 *   for the given f_stat.
 * Note: The mapping from primary_t to arg_type is not done here. Since the
 *   function calls are manually broken up via a switch statement anyways, it
 *   would have just added extra complexity to break it up even further.
 * Returns: true if primary evaluates to true, false if it doesn't or if the
 *   primary does not exist.
 */
bool primary_evaluate(primary_t primary, primary_arg *arg,\
        primary_args_g *globals, struct stat *f_stat) {
    bool ret = false;
    switch(primary) {
    case CNEWER:
        ret = eval_cnewer(f_stat, arg->stat_arg);
        break;
    case CMIN:
        ret = eval_cmin(f_stat, arg->long_arg, globals);
        break;
    case CTIME:
        ret = eval_ctime(f_stat, arg->long_arg, globals);
        break;
    case MMIN:
        ret = eval_mmin(f_stat, arg->long_arg, globals);
        break;
    case MTIME:
        ret = eval_mtime(f_stat, arg->long_arg, globals);
        break;
    case TYPE:
        ret = eval_type(f_stat, arg->char_arg);
        break;
    }
    return ret;
}

// After this point are all the primary evaluation functions and their helpers.

/**
 * Returns: true if f_stat is newer than o_stat, false otherwise.
 */
bool eval_cnewer(struct stat *f_stat, struct stat *o_stat) {
    return f_stat->st_ctim.tv_sec > o_stat->st_ctim.tv_sec || \
        (f_stat->st_ctim.tv_sec == o_stat->st_ctim.tv_sec && \
         f_stat->st_ctim.tv_nsec > o_stat->st_ctim.tv_nsec);
}

/**
 * Returns: true if the last change of file status information was n minutes
 *   ago (rounded up), false otherwise.
 */
bool eval_cmin(struct stat *f_stat, long n, primary_args_g *global_args) {
    time_t min = f_stat->st_ctim.tv_sec / SEC_PER_MIN;
    return global_args->time_min - min == n;
}

/**
 * Returns: true if the last change of file status information was n days
 *   ago (rounded up), false otherwise.
 */
bool eval_ctime(struct stat *f_stat, long n, primary_args_g *global_args) {
    time_t day = f_stat->st_ctim.tv_sec / SEC_PER_DAY;
    return global_args->time_day - day == n;
}

/**
 * Returns: true if the last file modification was n minutes ago (rounded up),
 *   false otherwise.
 */
bool eval_mmin(struct stat *f_stat, long n, primary_args_g *global_args) {
    time_t min = f_stat->st_mtim.tv_sec / SEC_PER_MIN;
    return global_args->time_min - min == n;
}

/**
 * Returns: true if the last file modification was n days ago (rounded up),
 *   false otherwise.
 */
bool eval_mtime(struct stat *f_stat, long n, primary_args_g *global_args) {
    time_t day = f_stat->st_mtim.tv_sec / SEC_PER_DAY;
    return global_args->time_day - day == n;
}

/**
 * Returns: true if the character representation of f_stat->st_mode is
 *   equivalent to t, false otherwise.
 */
bool eval_type(struct stat *f_stat, char t) {
    return get_type_char(f_stat->st_mode) == t;
}

/**
 * Returns the character representation of the filetype of mode, and a '?'
 *   if the filetype is invalid.
 */
char get_type_char(mode_t mode) {
    static const char type_char[] = {'b', 'c', 'd', 'f', 'l', 'p', 's', '?'};
    static const int type[] = {S_IFBLK, S_IFCHR, S_IFDIR, S_IFREG, S_IFLNK, \
        S_IFIFO, S_IFSOCK};
    
    mode &= S_IFMT;
    int i = 0;
    while (i < 7 && type[i] != mode) {
        i++;
    }

    return type_char[i];
}
