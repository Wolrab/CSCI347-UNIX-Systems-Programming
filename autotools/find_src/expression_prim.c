/**
 * TODO:
 */
#include "expression_prim.h"

/**
 * Takes a primary, its arg value, and globals, and returns its truth value
 *   for the given file tree entry.
 * To reduce individual evaluator function complexity the entries in entry are
 *   extracted inside here and subsequently passed to the appropriate
 *   evaluators. To make this as painless as possible, all evaluator functions
 *   should keep most of the naming semantics of the elements they directly 
 *   reference.
 * Returns: the truth value for the given primary.
 * In the case that the primary doesn't exist, the program aborts.
 */
bool primary_evaluate(primary_t primary, primary_arg *arg,\
        primary_args_g *globals, FTSENT *entry) {
    bool ret = false;
    switch(primary) {
    case CNEWER:
        ret = eval_cnewer(&(entry->fts_statp->st_ctim), arg->ctim_arg);
        break;
    case CMIN:
        ret = eval_cmin(&(entry->fts_statp->st_ctim), arg->long_arg, globals);
        break;
    case CTIME:
        ret = eval_ctime(&(entry->fts_statp->st_ctim), arg->long_arg, globals);
        break;
    case MMIN:
        ret = eval_mmin(&(entry->fts_statp->st_mtim), arg->long_arg, globals);
        break;
    case MTIME:
        ret = eval_mtime(&(entry->fts_statp->st_mtim), arg->long_arg, globals);
        break;
    case TYPE:
        ret = eval_type(entry->fts_statp->st_mode, arg->char_arg);
        break;
    case EXEC:
        ret = eval_exec(entry->fts_path, arg->argv_arg);
        break;
    case PRIMARY_NUM:
        abort();
    default:
        abort();
    }
    return ret;
}

// Primary evaluation functions
/**
 * Returns true if f_stat is newer than o_stat, false otherwise.
 */
bool eval_cnewer(struct timespec *ctim, struct timespec *o_ctim) {
    return ctim->tv_sec > o_ctim->tv_sec || \
        (ctim->tv_sec == o_ctim->tv_sec && \
         ctim->tv_nsec > o_ctim->tv_nsec);
}

/**
 * Returns true if the last change of file status information was n minutes
 *   ago (rounded up), false otherwise.
 */
bool eval_cmin(struct timespec *ctim, long n, primary_args_g *global_args) {
    time_t min = ctim->tv_sec / SEC_PER_MIN;
    return global_args->time_min - min == n;
}

/**
 * Returns true if the last change of file status information was n days
 *   ago (rounded up), false otherwise.
 */
bool eval_ctime(struct timespec *ctim, long n, primary_args_g *global_args) {
    time_t day = ctim->tv_sec / SEC_PER_DAY;
    return global_args->time_day - day == n;
}

/**
 * Returns true if the last file modification was n minutes ago (rounded up),
 *   false otherwise.
 */
bool eval_mmin(struct timespec *mtim, long n, primary_args_g *global_args) {
    time_t min = mtim->tv_sec / SEC_PER_MIN;
    return global_args->time_min - min == n;
}

/**
 * Returns true if the last file modification was n days ago (rounded up),
 *   false otherwise.
 */
bool eval_mtime(struct timespec *mtim, long n, primary_args_g *global_args) {
    time_t day = mtim->tv_sec / SEC_PER_DAY;
    return global_args->time_day - day == n;
}

/**
 * Returns true if the character representation of f_stat->st_mode is
 *   equivalent to t, false otherwise.
 */
bool eval_type(mode_t mode, char t) {
    return get_type_char(mode) == t;
}

/**
 * TODO: This is not a cut and dry fucking true or false if shit goes sideways.
 */
 #include <stdio.h>
bool eval_exec(char *path, argv_t argv) {
    pid_t pid;
    int status;

    pid = fork();
    if (pid == 0) {
        printf("%s\n", argv[0]);
        execvp(argv[0], &(argv[0]));
        return -1;
    }
    else if (waitpid(pid, &status, 0) == -1) {
        status = -1;
    }
    return status == 0;
}

// Helpers for primary evaluation functions
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
