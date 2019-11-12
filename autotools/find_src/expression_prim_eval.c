/**
 * Interface for evaluating expression primaries. To evaluate a primary, the
 *   only function that needs to be called is primary_evaluate, as it properly
 *   associates each primary_t with the appropriate evaluator function and has
 *   extra assertions to ensure each primaries arg was appropriately parsed.
 */
#include "expression_prim_eval.h"

/**
 * Takes a primary, its arg value, the programs state and FTSENT struct for a
 *   file, and returns the primary's truth value for the given file tree entry.
 * To localize most of the implementation details, all relevant arguments for a
 *   given primary are unwrapped here and passed in to each primary evaluator
 *   function. This also means asserts can be included as one final check that
 *   the mapping used by primary_arg_parse was correct and we can be sure our
 *   dereference of arg is the correct data type.
 * In the case that the primary doesn't exist, the program aborts.
 */
bool primary_evaluate(primary_t primary, primary_arg *arg,\
        prog_state *state_args, FTSENT *entry) {
    bool ret = false;
    switch(primary) {
    case CNEWER:
        assert(primary_arg_type_map[primary] == CTIM_ARG);
        ret = eval_cnewer(&(entry->fts_statp->st_ctim), arg->ctim_arg);
        break;
    case CMIN:
        assert(primary_arg_type_map[primary] == LONG_ARG);
        ret = eval_cmin(&(entry->fts_statp->st_ctim), arg->long_arg, \
            state_args->start_time_min);
        break;
    case CTIME:
        assert(primary_arg_type_map[primary] == LONG_ARG);
        ret = eval_ctime(&(entry->fts_statp->st_ctim), arg->long_arg, \
            state_args->start_time_day);
        break;
    case MMIN:
        assert(primary_arg_type_map[primary] == LONG_ARG);
        ret = eval_mmin(&(entry->fts_statp->st_mtim), arg->long_arg, \
            state_args->start_time_min);
        break;
    case MTIME:
        assert(primary_arg_type_map[primary] == LONG_ARG);
        ret = eval_mtime(&(entry->fts_statp->st_mtim), arg->long_arg, \
            state_args->start_time_min);
        break;
    case TYPE:
        assert(primary_arg_type_map[primary] == CHAR_ARG);
        ret = eval_type(entry->fts_statp->st_mode, arg->char_arg);
        break;
    case EXEC:
        assert(primary_arg_type_map[primary] == ARGV_ARG);
        ret = eval_exec(entry->fts_path, arg->argv_arg->argv, \
            arg->argv_arg->argv_dest, arg->argv_arg->argc);
        break;
    case PRIMARY_NUM:
        abort();
    default:
        abort();
    }
    return ret;
}

// Primary evaluator functions. The use of primary_evaluate allows for
//   meaningful and descriptive function signatures for each function.

/**
 * Returns true if the file status change time ctim is greater than the file
 *   status change time o_ctim. Otherwise returns false. The granularity of the
 *   comparison is in nanoseconds.
 */
bool eval_cnewer(struct timespec *ctim, struct timespec *o_ctim) {
    return ctim->tv_sec > o_ctim->tv_sec || \
        (ctim->tv_sec == o_ctim->tv_sec && ctim->tv_nsec > o_ctim->tv_nsec);
}

/**
 * Returns true if the file status change time was n minutes ago (rounded up).
 *   Otherwise returns false. The granularity of the comparison is in seconds.
 */
bool eval_cmin(struct timespec *ctim, long n, time_t start_time_min) {
    time_t min = ctim->tv_sec / SEC_PER_MIN;
    return start_time_min - min == n;
}

/**
 * Returns true if the file status change time was n days ago (rounded up).
 *   Otherwise returns false. The granularity of the comparison is in seconds.
 */
bool eval_ctime(struct timespec *ctim, long n, time_t start_time_day) {
    time_t day = ctim->tv_sec / SEC_PER_DAY;
    return start_time_day - day == n;
}

/**
 * Returns true if the file modification time was n minutes ago (rounded up).
 *   Otherwise returns false. The granularity of the comparison is in seconds.
 */
bool eval_mmin(struct timespec *mtim, long n, time_t start_time_min) {
    time_t min = mtim->tv_sec / SEC_PER_MIN;
    return start_time_min - min == n;
}

/**
 * Returns true if the file modification time was n days ago (rounded up).
 *   Otherwise returns false. The granularity of the comparison is in seconds.
 */
bool eval_mtime(struct timespec *mtim, long n, time_t start_time_day) {
    time_t day = mtim->tv_sec / SEC_PER_DAY;
    return start_time_day - day == n;
}

/**
 * Returns true if the character representation of the file type of mode is
 *   equivalent to t. Otherwise returns false.
 */
bool eval_type(mode_t mode, char t) {
    return get_type_char(mode) == t;
}

/**
 * Returns true if the program executed with argv returns 0. Otherwise returns
 *   false. Any element of argv that is equivalent to the string
 *   PRIM_EXEC_PATH_EXPAND is replaced by path.
 * argv_dest is included to make the job of string replacement easier as without
 *   it a new array would have to be allocated every call. argv_dest is made
 *   valid by the writting and is only read by the new process.
 */
bool eval_exec(char *path, char **argv, char **argv_dest, int argc) {
    pid_t pid;
    int status;

    for(int i = 0; i < argc; i++) {
        if (strncmp(argv[i], PRIM_EXEC_PATH_EXPAND, \
                strlen(PRIM_EXEC_PATH_EXPAND) + 1)==0) {
            argv_dest[i] = path;
        }
        else {
            argv_dest[i] = argv[i];
        }
    }
    argv_dest[argc] = NULL;

    pid = fork();
    if (pid == 0) {
        int fd_in = open("/dev/null", O_RDONLY);
        int fd_out = open("/dev/null", O_WRONLY);
        int fd_err = open("/dev/null", O_WRONLY);
        dup2(fd_in, STDIN_FILENO);
        dup2(fd_out, STDOUT_FILENO);
        dup2(fd_err, STDERR_FILENO);
        execvp(argv_dest[0], argv_dest);
        return -1;
    }
    else if (waitpid(pid, &status, 0) == -1) {
        status = -1;
    }
    return status == 0;
}

/**
 * Returns the character representation of the filetype of mode, and a '?'
 *   if the filetype is invalid.
 */
char get_type_char(mode_t mode) {
    static const char type_char[] = {'b', 'c', 'd', 'f', 'l', 'p', 's', '?'};
    static const int type[] = {S_IFBLK, S_IFCHR, S_IFDIR, S_IFREG, S_IFLNK, \
        S_IFIFO, S_IFSOCK};
    static const int type_c = 7;
    
    mode &= S_IFMT;
    int i = 0;
    while (i < type_c && type[i] != mode) {
        i++;
    }

    return type_char[i];
}
