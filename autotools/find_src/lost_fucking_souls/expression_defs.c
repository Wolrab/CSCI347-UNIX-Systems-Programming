#include "expression_defs.h"

// Valid characters for the file_type arg_type
const char *const valid_file_types = "bcdflps";
// String equivalents of corresponding primaries
const char *const primary_str_arr[] = {"-cnewer", "-cmin", "-ctime", "-mmin", \
    "-mtime", "-type"};
// arg_type's of corresponding primaries
const arg_type primary_arg_type_arr[] = {ARG_STAT, ARG_LONG, ARG_LONG, \
    ARG_LONG, ARG_LONG, ARG_FILE_TYPE};

/**
 * Sets the global values for time comparison. This must be only called once, 
 *   no idea how to ensure that though.
 */
void set_start_time(time_t prog_start) {
    start_time_day = prog_start / SEC_PER_DAY;
    if (prog_start % SEC_PER_DAY > 0) {
        start_time_day++;
    }
    
    start_time_min = prog_start / SEC_PER_MIN;
    if (prog_start % SEC_PER_MIN > 0) {
        start_time_min++;
    }
}

/**
 * eval function for primary cnewer
 */
bool eval_cnewer(struct stat *f_stat, struct stat *o_stat) {
    if (f_stat->st_ctim.tv_sec - o_stat->st_ctim.tv_sec > 0) {
        return true;
    }
    if (f_stat->st_ctim.tv_sec - o_stat->st_ctim.tv_sec == 0 && \
        f_stat->st_ctim.tv_nsec - o_stat->st_ctim.tv_nsec > 0) {
        return true;
    }
    return false;
}

/**
 * eval function for primary cmin
 */
bool eval_cmin(struct stat *f_stat, long n) {
    time_t min = f_stat->st_ctim.tv_sec / SEC_PER_MIN;
    if (start_time_min - min == n) {
        return true;
    }
    return false;
}

/**
 * eval function for primary ctime
 */
bool eval_ctime(struct stat *f_stat, long n) {
    time_t day = f_stat->st_ctim.tv_sec / SEC_PER_DAY;
    if (start_time_day - day == n) {
        return true;
    }
    return false;
}

/**
 * eval function for primary mmin
 */
bool eval_mmin(struct stat *f_stat, long n) {
    time_t min = f_stat->st_mtim.tv_sec / SEC_PER_MIN;
    if (start_time_min - min == n) {
        return true;
    }
    return false;
}

/**
 * eval function for primary mtime
 */
bool eval_mtime(struct stat *f_stat, long n) {
    time_t day = f_stat->st_mtim.tv_sec / SEC_PER_DAY;
    if (start_time_day - day == n) {
        return true;
    }
    return false;
}

/**
 * eval function for primary type
 */
bool eval_type(struct stat *f_stat, char t) {
    if (get_type_char(f_stat->st_mode) == t) {
        return true;
    }
    return false;
}

/**
 * Returns mode characters the mega-sinful way.
 */
char get_type_char(mode_t mode) {
    if (S_ISBLK(mode)) {
        return 'b';
    }
    if (S_ISCHR(mode)) {
        return 'c';
    }
    if (S_ISDIR(mode)) {
        return 'd';
    }
    if (S_ISREG(mode)) {
        return 'f';
    }
    if (S_ISLNK(mode)) {
        return 'l';
    }
    if (S_ISFIFO(mode)) {
        return 'p';
    }
    if (S_ISSOCK(mode)) {
        return 's';
    }
    return '?';
}
