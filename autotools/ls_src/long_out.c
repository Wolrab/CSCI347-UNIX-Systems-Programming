/**
 * Functions to handle the long format of output for ls. It converts all
 *   relevant input from a stat structure to formatted strings, and then prints
 *   the result using long_out_print.
 */
#include "long_out.h"

/**
 * Fills long_out using the fields given in data.
 * Returns: 0 on success, -1 if the parsing failed.
 */
int long_out_parse(struct long_out_s *long_out, struct stat *f_stat, \
        char *f_name) {
    int ret = 0;
    ret = parse_usr_str(&(long_out->usr_str), f_stat->st_uid);
    if (ret == 0) {
        ret = parse_grp_str(&(long_out->grp_str), f_stat->st_gid);
        if (ret < 0) {
            free(long_out->usr_str);
        }
        else {
            ret = parse_mtim_str(long_out->mtim_str, f_stat->st_mtime);
            if (ret < 0) {
                free(long_out->usr_str);
                free(long_out->grp_str);
            }
            else {
                parse_mode_str(long_out->mode_str, f_stat->st_mode);
                long_out->nlink = f_stat->st_nlink;
                long_out->size = f_stat->st_size;
                long_out->f_name = f_name;
            }
        }
    }
    
    return ret;
}

/**
 * Fills a fixed sized character sequence representing important data from mode.
 */
void parse_mode_str(char *mode_s, mode_t mode) {
    mode_s[0] = get_type_char(mode);
    mode_s[1] = S_IRUSR & mode ? 'r' : '-';
    mode_s[2] = S_IWUSR & mode ? 'w' : '-';
    mode_s[3] = S_IXUSR & mode ? 'x' : '-';
    mode_s[4] = S_IRGRP & mode ? 'r' : '-';
    mode_s[5] = S_IWGRP & mode ? 'w' : '-';
    mode_s[6] = S_IXGRP & mode ? 'x' : '-';
    mode_s[7] = S_IROTH & mode ? 'r' : '-';
    mode_s[8] = S_IWOTH & mode ? 'w' : '-';
    mode_s[9] = S_IXOTH & mode ? 'x' : '-';
    mode_s[10] = '\0';
}

/**
 * Allocates and sets the value pointed at by usr_str to be the user name 
 *   of uid.
 * Returns: 0 on success, -1 if the parsing failed.
 */
int parse_usr_str(char **usr_str, uid_t uid) {
    struct passwd *passwd_ent;
    char num_buf[32];
    int ret = 0;

    passwd_ent = getpwuid(uid);
    if (passwd_ent == NULL) {
        sprintf(num_buf, "%d", uid);
        errno = 0;
        *usr_str = malloc(strlen(num_buf)+1);
        if (*usr_str == NULL && errno) {
            ret = -1;
        }
        else {
            memcpy(*usr_str, num_buf, strlen(num_buf)+1);
        }
    }
    else {
        errno = 0;
        *usr_str = malloc(strlen(passwd_ent->pw_name) + 1);
        if (*usr_str == NULL && errno) {
            ret = -1;
        }
        else {
            memcpy(*usr_str, passwd_ent->pw_name, \
                strlen(passwd_ent->pw_name) + 1);
        }
    }
    return ret;
}

/**
 * Allocates and sets the value pointed at by grp_str to be the group name 
 *   of gid.
 * Returns: 0 on success, -1 if the parsing failed.
 */
int parse_grp_str(char **grp_str, gid_t gid) {
    struct group *group_ent;
    char num_buf[32];
    int ret = 0;

    group_ent = getgrgid(gid);
    if (group_ent == NULL) {
        sprintf(num_buf, "%ud", gid);
        errno = 0;
        *grp_str = malloc(strlen(num_buf)+1);
        if (*grp_str == NULL && errno) {
            ret = -1;
        }
        else {
            memcpy(*grp_str, num_buf, strlen(num_buf)+1);
        }
    }
    else {
        errno = 0;
        *grp_str = malloc(strlen(group_ent->gr_name) + 1);
        if (*grp_str == NULL && errno) {
            ret = -1;
        }
        else {
            memcpy(*grp_str, group_ent->gr_name, \
                strlen(group_ent->gr_name) + 1);
        }
    }
    return ret;
}

/** 
 * Gets the formatted date from mtim.
 * Returns: 0 on success, -1 if the parsing failed.
 */
int parse_mtim_str(char *mtim_str, time_t mtim) {
    struct tm *t;
    int ret = 0;
    
    t = localtime(&mtim);
    errno = 0;
    if (strftime(mtim_str, DATE_STR_LEN, "%b %e %H:%M", t) == 0) {
        ret = -1;
    }
    
    return ret;
}

/**
 * Prints a long format ls entry given long_out.
 */
void long_out_print(struct long_out_s *long_out) {
    printf("%s %u %s %s %lld %s %s\n", long_out->mode_str, long_out->nlink,
        long_out->usr_str, long_out->grp_str, long_out->size,
        long_out->mtim_str, long_out->f_name);
}

/**
 * Frees all dynamically allocated fields of long_out.
 */
void long_out_delete(struct long_out_s *long_out) {
    free(long_out->usr_str);
    free(long_out->grp_str);
}

/**
 * Returns the character representation of the filetype of mode, and a '?'
 *   if the filetype is invalid.
 */
char get_type_char(mode_t mode) {
    static const char type_char[] = {'b', 'c', 'd', '-', 'l', 'p', 's', '?'};
    static const int type[] = {S_IFBLK, S_IFCHR, S_IFDIR, S_IFREG, S_IFLNK, \
        S_IFIFO, S_IFSOCK};
    
    mode &= S_IFMT;
    int i = 0;
    while (i < 7 && type[i] != mode) {
        i++;
    }

    return type_char[i];
}