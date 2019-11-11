/**
 * Functions to handle the long format of output for ls. It converts all
 *   relevant input from a stat structure to formatted strings, and then prints
 *   the result using long_out_print.
 */
#include "long_out.h"

/**
 * TODO:
 */
void dir_long_out_init(struct dir_long_out_s *dir_long_out) {
    dir_long_out->ino_str_max   = 0;
    dir_long_out->nlink_str_max = 0;
    dir_long_out->usr_str_max   = 0;
    dir_long_out->grp_str_max   = 0;
    dir_long_out->size_str_max  = 0;
    dir_long_out->entries   = NULL;
    dir_long_out->entries_c = 0;
}

/**
 * TODO:
 * 
 */
int dir_long_out_create(struct dir_long_out_s *dir_long_out, 
        list *dir_entries) {
    node *curr = NULL;
    int ret = 0;

    dir_long_out->entries = malloc(sizeof(struct long_out_s) * \
        dir_entries->size);
    if (dir_long_out->entries && errno) {
        ret = -1;
    }
    else {
        curr = dir_entries->head;
        int i = 0;
        while (ret == 0 && curr != NULL && i < dir_entries->size) {
            ret = long_out_parse(&(dir_long_out->entries[i]), \
                curr->data.f_name, curr->data.f_stat);
            if (ret < 0) {
                dir_long_out_delete(dir_long_out);
            }
            else {
                dir_long_out->entries_c++;
                curr = curr->next;
                i++;
            }
        }
        if (ret == 0) {
            assert(curr == NULL && i == dir_entries->size);
            set_max_strs(dir_long_out);
        }
    }
    return ret;
}

void set_max_strs(struct dir_long_out_s *dir_long_out) {
    for (int i = 0; i < dir_long_out->entries_c; i++) {
        if (strlen(dir_long_out->entries[i].ino_str) > \
                dir_long_out->ino_str_max) {
            dir_long_out->ino_str_max = \
                strlen(dir_long_out->entries[i].ino_str);
        }
        if (strlen(dir_long_out->entries[i].nlink_str) > \
                dir_long_out->nlink_str_max) {
            dir_long_out->nlink_str_max = \
                strlen(dir_long_out->entries[i].nlink_str);
        }
        if (strlen(dir_long_out->entries[i].usr_str) > \
                dir_long_out->usr_str_max) {
            dir_long_out->usr_str_max = \
                strlen(dir_long_out->entries[i].usr_str);
        }
        if (strlen(dir_long_out->entries[i].grp_str) > \
                dir_long_out->grp_str_max) {
            dir_long_out->grp_str_max = \
                strlen(dir_long_out->entries[i].grp_str);
        }
        if (strlen(dir_long_out->entries[i].size_str) > \
                dir_long_out->size_str_max) {
            dir_long_out->size_str_max = \
                strlen(dir_long_out->entries[i].size_str);
        }
    }
}

/**
 * Fills long_out using the fields given in data. Calls parse_misc_st_str last
 *   so there is no need to remove all those elements on a secondary failure.
 * Returns 0 on success, -1 if the parsing failed.
 */
int long_out_parse(struct long_out_s *long_out, char *f_name, \
        struct stat *f_stat) {
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
                ret = parse_misc_st_str(long_out, f_stat);
                if (ret < 0) {
                    free(long_out->usr_str);
                    free(long_out->grp_str);
                }
                else {
                    parse_mode_str(long_out->mode_str, f_stat->st_mode);
                    long_out->f_name = f_name;
                }
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
 *   of uid. If uid has no corresponding username, the string representation of
 *   uid is stored instead.
 * Returns 0 on success, -1 if the parsing failed.
 */
int parse_usr_str(char **usr_str, uid_t uid) {
    struct passwd *passwd_ent;
    char *uid_str;
    int ret = 0;

    passwd_ent = getpwuid(uid);
    if (passwd_ent == NULL) {
        errno = 0;
        uid_str = malloc(get_f_max_strlen(UID_PRINTF));
        if (uid_str == NULL) {
            ret = -1;
        }
        else {
            sprintf(uid_str, UID_PRINTF, uid);

            errno = 0;
            *usr_str = malloc(strlen(uid_str)+1);
            if (*usr_str == NULL && errno) {
                ret = -1;
            }
            else {
                strncpy(*usr_str, uid_str, strlen(uid_str)+1);
            }
            free(uid_str);
        }
    }
    else {
        errno = 0;
        *usr_str = malloc(strlen(passwd_ent->pw_name) + 1);
        if (*usr_str == NULL && errno) {
            ret = -1;
        }
        else {
            strncpy(*usr_str, passwd_ent->pw_name, \
                strlen(passwd_ent->pw_name) + 1);
        }
    }
    return ret;
}

/**
 * Allocates and sets the value pointed at by grp_str to be the group name 
 *   of gid. If gid has no corresponding group name, the string representation 
 *   of gid is stored instead.
 * Returns 0 on success, -1 if the parsing failed.
 */
int parse_grp_str(char **grp_str, gid_t gid) {
    struct group *group_ent;
    char *gid_str;
    int ret = 0;

    group_ent = getgrgid(gid);
    if (group_ent == NULL) {
        errno = 0;
        gid_str = malloc(get_f_max_strlen(GID_PRINTF));
        if (gid_str == NULL) {
            ret = -1;
        }
        else {
            sprintf(gid_str, GID_PRINTF, gid);

            errno = 0;
            *grp_str = malloc(strlen(gid_str)+1);
            if (*grp_str == NULL && errno) {
                ret = -1;
            }
            else {
                strncpy(*grp_str, gid_str, strlen(gid_str)+1);
            }
            free(gid_str);
        }
    }
    else {
        errno = 0;
        *grp_str = malloc(strlen(group_ent->gr_name) + 1);
        if (*grp_str == NULL && errno) {
            ret = -1;
        }
        else {
            strncpy(*grp_str, group_ent->gr_name, \
                strlen(group_ent->gr_name) + 1);
        }
    }
    return ret;
}

/** 
 * Gets the formatted date from mtim.
 * Returns 0 on success, -1 if the parsing failed.
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
 * Parses all members of f_stat to strings who need no special formatting and
 *   puts them in long_out.
 * Returns 0 on success, -1 on error.
 */
int parse_misc_st_str(struct long_out_s *long_out, struct stat *f_stat) {
    int ret = 0;

    errno = 0;
    long_out->ino_str = malloc(get_f_max_strlen(INO_PRINTF));
    if (long_out->ino_str == NULL) {
        ret = -1;
    }
    else {
        sprintf(long_out->ino_str, INO_PRINTF, f_stat->st_ino);

        errno = 0;
        long_out->nlink_str = malloc(get_f_max_strlen(NLINK_PRINTF));
        if (long_out->nlink_str == NULL) {
            free(long_out->ino_str);
            ret = -1;
        }
        else {
            sprintf(long_out->nlink_str, NLINK_PRINTF, f_stat->st_nlink);

            errno = 0;
            long_out->size_str = malloc(get_f_max_strlen(OFF_PRINTF));
            if (long_out->size_str == NULL) {
                free(long_out->ino_str);
                free(long_out->nlink_str);
                ret = -1;
            }
            else {
                sprintf(long_out->size_str, OFF_PRINTF, f_stat->st_size);
            }
        }
    }
    
    return ret;
}

/**
 * Prints the given list of entries in long-format output using each entrie's
 *   parsed long_out_s struct and formatting informtion from dir_long_out. If
 *   option_i is true, it will also prepend each entry with its i-node number.
 */
void dir_long_out_print(struct dir_long_out_s *dir_long_out, bool option_i) {
    struct long_out_s *curr_ent;
    for (int i = 0; i < dir_long_out->entries_c; i++) {
        curr_ent = &(dir_long_out->entries[i]);
        if (option_i) {
            printf("%*s ", dir_long_out->ino_str_max, curr_ent->ino_str);
        }
        printf("%s %*s %*s %*s %*s %s %s\n",                  \
            curr_ent->mode_str,                               \
            dir_long_out->nlink_str_max, curr_ent->nlink_str, \
            dir_long_out->usr_str_max,   curr_ent->usr_str,   \
            dir_long_out->grp_str_max,   curr_ent->grp_str,   \
            dir_long_out->size_str_max,  curr_ent->size_str,  \
            curr_ent->mtim_str,                               \
            curr_ent->f_name);
    }
}

/**
 * TODO:
 */
void dir_long_out_delete(struct dir_long_out_s *dir_long_out) {
    for (int i = 0; i < dir_long_out->entries_c; i++) {
        long_out_delete(&(dir_long_out->entries[i]));
    }
    free(dir_long_out->entries);
    dir_long_out_init(dir_long_out);
}

/**
 * Frees all elements of long_out
 */
void long_out_delete(struct long_out_s *long_out) {
    free(long_out->usr_str);
    free(long_out->grp_str);
    free(long_out->ino_str);
    free(long_out->nlink_str);
    free(long_out->size_str);
}

/**
 * Returns the character representation of the filetype of mode, and a '?'
 *   if the filetype is invalid.
 */
char get_type_char(mode_t mode) {
    static const char type_char[] = {'b', 'c', 'd', '-', 'l', 'p', 's', '?'};
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