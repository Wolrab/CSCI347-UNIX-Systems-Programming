#include "ls.h"

// Basic error message defines to make ls_perror easier on the eyes
const char *const ls_err_msg[] = {
    "no error",
    "memory allocation error",
    "error opening a directory stream",
    "error reading a directory stream",
    "error getting file statistics",
    "warning: duplicate entry found",
    "path overflows maximum path length",
    "date overflows expected length",
    "user name not found",
    "group name not found"
};

/** 
 * Entry point for ls. Ensures arguments are defined and then calls ls
 *   functionality on the CWD. If an error occurs, its message is then printed.
 * Returns: 0 on success, >0 on error
 */
int main(int argc, char **argv) {
    ls_err ret = LS_ERR_NONE;

    ret = get_options(argc, argv);
    if (ret < 0) {
        printf("Usage: ls [-%s]\n", OPTION_STRING);
        return ret;
    }

    ret = ls(".");
    if (ret != LS_ERR_NONE) {
        ls_perror(ret, argv[0]);
    }

    return ret;
}

/**
 * Prints the entries of the directory specified by path to stdout.
 * Returns: LS_ERR_NONE on success, and on error an ls_err value corresponding
 *   to the type of error.
 */
ls_err ls(char *path) {
    list dir_entries = NULL;
    ls_err ret = LS_ERR_NONE;

    ret = get_entries(path, &dir_entries);
    if (ret != LS_ERR_NONE) {
        list_delete(&dir_entries);
        return ret;
    }

    if (option_l) {
        ret = output_ent_stats(&dir_entries);
    }
    else {
        output_ent_names(&dir_entries);
    }
    
    list_delete(&dir_entries);
    return ret;
}

/**
 * Open and iterates through a directory stream at path, filling dir_entries 
 *   with each entry.
 * Returns: LS_ERR_NONE on success, and on error an ls_err value corresponding
 *   to the type of error.
 */
ls_err get_entries(const char *path, list *dir_entries) {
    DIR *d = NULL;
    struct dirent *ent = NULL;
    ls_err ret = LS_ERR_NONE;

    errno = 0;
    d = opendir(path);
    if (d == NULL) {
        return LS_ERR_MALLOC;
    }

    errno = 0;
    ent = readdir(d);
    while (ent != NULL) {
        if (!(option_a) && ent->d_name[0] == '.') {
            errno = 0;
            ent = readdir(d);
            continue;
        }

        ret = parse_entry(ent, path, dir_entries);
        if (ret != LS_ERR_NONE && ret != LS_ERR_DUP_ENTRY) {
            return ret;
        }

        errno = 0;
        ent = readdir(d);
    }
    if (ent == NULL && errno) {
        ret = LS_ERR_DIR_STREAM_READ;
    }

    closedir(d);
    return ret;
}

/** 
 * Parses the data in ent and adds a new node with the parsed data into 
 *   dir_entries.
 * Returns: LS_ERR_NONE on success, and on error an ls_err value corresponding
 *   to the type of error.
 */
ls_err parse_entry(struct dirent *ent, const char *path, \
        list *dir_entries) {
    node *ent_node = NULL;

    struct stat *ent_stat = NULL;
    char stat_path[PATH_MAX];

    list_err err = LIST_ERR_NONE;
    ls_err ret = LS_ERR_NONE;

    if (option_l) {
        ret = get_full_path(stat_path, PATH_MAX, ent->d_name, path);
        if (ret == LS_ERR_PATH_OVERFLOW) {
            return ret;
        }

        errno = 0;
        ent_stat = malloc(sizeof(struct stat));
        if (ent_stat == NULL) {
            return LS_ERR_MALLOC;
        }

        if (lstat(stat_path, ent_stat) < 0) {
            free(ent_stat);
            return LS_ERR_STAT;
        }
    }

    ent_node = list_create_node(ent->d_name, ent_stat);
    if (ent_node == NULL) {
        return LS_ERR_MALLOC;
    }

    err = list_insert_ordered(dir_entries, ent_node);
    if (err == LIST_ERR_DUP_ENTRY) {
        ret = LS_ERR_DUP_ENTRY;
    }

    return ret;
}

/** 
 * Gets the full path of f_name given path and a buffer to store the 
 *   new path.
 * Returns: LS_ERR_NONE on success and LS_ERR_PATH_OVERFLOW if the concatenated
 *   path would overflow path_buf.
 */
ls_err get_full_path(char *path_buf, int path_buf_len, const char *f_name, \
        const char *path) {
    ls_err ret = LS_ERR_NONE;

    if (strlen(path) + strlen(f_name) + 1 > path_buf_len) {
        return LS_ERR_PATH_OVERFLOW;
    }

    memcpy(path_buf, path, strlen(path) + 1);
    if (path_buf[strlen(path)-1] != '/') {
        path_buf[strlen(path)+1] = '\0';
        path_buf[strlen(path)] = '/';
    }
    memcpy(path_buf + strlen(path_buf), f_name, strlen(f_name) + 1);

    return ret;
}

/**
 * Outputs the filenames stored in dir_entries to stdout
 */
void output_ent_names(list *dir_entries) {
    node *curr;

    curr = *dir_entries;
    while (curr != NULL) {
        printf("%s\n", curr->data.f_name);
        curr = curr->next;
    }
}

/**
 * Outputs the filenames and stat information in dir_entries to stdout. asserts
 *   that each entry has a non-null f_stat field.
 * Returns: LS_ERR_NONE on success, and any string-conversion related error if
 *   any conversion failed.
 */
ls_err output_ent_stats(list *dir_entries) {
    struct stat_out_s stat_out;
    node *curr;
    ls_err ret = LS_ERR_NONE;

    unsigned long nlink_cast;
    long size_cast;
    
    curr = *dir_entries;
    while (curr != NULL) {
        assert(curr->data.f_stat != NULL);

        ret = fill_stat_out(&stat_out, &(curr->data));
        if (ret != LS_ERR_NONE) {
            return ret;
        }

        nlink_cast = (unsigned long)curr->data.f_stat->st_nlink;
        size_cast = (long)curr->data.f_stat->st_size;

        printf("%s %lu %s %s %ld %s %s\n", stat_out.mode_str, 
            nlink_cast, stat_out.usr_str, stat_out.grp_str, 
            size_cast, stat_out.mtim_str, stat_out.f_name);

        free_stat_out(&stat_out);
        curr = curr->next;
    }
    return ret;
}

/**
 * Fills stat_out using the fields given in data.
 * Returns: LS_ERR_NONE on success, otherwise any of the errors from the 
 *   get_*_str functions and all data is freed.
 */
ls_err fill_stat_out(struct stat_out_s *stat_out, struct data_s *data) {
    ls_err ret = LS_ERR_NONE;

    get_mode_str(stat_out->mode_str, data->f_stat->st_mode);

    ret = get_usr_str(&(stat_out->usr_str), data->f_stat->st_uid);
    if (ret != LS_ERR_NONE) {
        goto exit;
    }

    ret = get_grp_str(&(stat_out->grp_str), data->f_stat->st_gid);
    if (ret != LS_ERR_NONE) {
        goto cleanup_usr;
    }
    
    ret = get_mtim_str(stat_out->mtim_str, data->f_stat->st_mtime);
    if (ret != LS_ERR_NONE) {
        goto cleanup_group_usr;
    }

    stat_out->f_name = data->f_name;
    return ret;

    cleanup_group_usr:
    free(stat_out->grp_str);
    cleanup_usr:
    free(stat_out->usr_str);
    exit:
    return ret;
}

/**
 * Fills a fixed sized character sequence representing important data from mode.
 */
void get_mode_str(char *mode_s, mode_t mode) {
    if (S_ISREG(mode))
        mode_s[0] = '-';
    else if (S_ISDIR(mode))
        mode_s[0] = 'd';
    else if (S_ISLNK(mode))
        mode_s[0] = 'l';
    else if (S_ISFIFO(mode))
        mode_s[0] = 'p';
    else if (S_ISSOCK(mode))
        mode_s[0] = 's';
    else if (S_ISCHR(mode))
        mode_s[0] = 'c';
    else if (S_ISBLK(mode))
        mode_s[0] = 'b';

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
 * Returns: LS_ERR_NONE on success, LS_ERR_USR_NOT_FOUND if the user name 
 *   wasn't found and LS_ERR_MALLOC if malloc fails.
 */
ls_err get_usr_str(char **usr_str, uid_t uid) {
    struct passwd *passwd_ent;
    ls_err ret = LS_ERR_NONE;

    passwd_ent = getpwuid(uid);
    if (passwd_ent == NULL) {
        return LS_ERR_USR_NOT_FOUND;
    }

    errno = 0;
    *usr_str = malloc(strlen(passwd_ent->pw_name) + 1);
    if (*usr_str == NULL && errno) {
        return LS_ERR_MALLOC;
    }

    memcpy(*usr_str, passwd_ent->pw_name, strlen(passwd_ent->pw_name) + 1);

    return ret;
}

/**
 * Allocates and sets the value pointed at by grp_str to be the group name 
 *   of gid.
 * Returns: LS_ERR_NONE on success, LS_ERR_GRP_NOT_FOUND if the group name 
 *   wasn't found and LS_ERR_MALLOC if malloc fails.
 */
ls_err get_grp_str(char **grp_str, gid_t gid) {
    struct group *group_ent;
    ls_err ret = LS_ERR_NONE;

    group_ent = getgrgid(gid);
    if (group_ent == NULL) {
        return LS_ERR_GRP_NOT_FOUND;
    }

    errno = 0;
    *grp_str = malloc(strlen(group_ent->gr_name) + 1);
    if (*grp_str == NULL && errno) {
        return LS_ERR_MALLOC;
    }

    memcpy(*grp_str, group_ent->gr_name, strlen(group_ent->gr_name) + 1);

    return ret;
}

/** 
 * Gets the formatted date from mtim.
 * Returns: LS_ERR_NONE on success, LS_ERR_DATE_OVERFLOW if the date could not
 *   fit inside mtim_str.
 */
ls_err get_mtim_str(char *mtim_str, time_t mtim) {
    struct tm *t;
    ls_err ret = LS_ERR_NONE;
    
    t = localtime(&mtim);
    errno = 0;
    if (strftime(mtim_str, DATE_STR_LEN, "%b %e %H:%M", t) == 0) {
        ret = LS_ERR_DATE_OVERFLOW;
    }
    
    return ret;
}

/**
 * Frees all dynamically allocated fields of stat_out
 */
void free_stat_out(struct stat_out_s *stat_out) {
    free(stat_out->usr_str);
    free(stat_out->grp_str);
}

/**
 * Checks argv for options and sets option flags.
 * Returns: 0 on success, -1 if an invalid option was found.
 */
int get_options(const int argc, char **argv) {
    char opt = -1;
    int ret = 0;
    
    opt = getopt(argc, argv, OPTION_STRING);
    while (opt != -1) {
        switch (opt) {
        case 'a':
            option_a = true;
            break;
        case 'l':
            option_l = true;
            break;
        case '?':
            ret = -1;
        }
        opt = getopt(argc, argv, OPTION_STRING);
    }
    return ret;
}

/**
 * Outputs any errors using each err's corresponding ls_err_message entry.
 *   In standard linux fashion, the program's name is prefixed to the error 
 *   output for clearer messages when chains of programs are piped into one
 *   another.
 * If errno is set, this prints using perror. This errno value should be 
 *   associated with the err actually given, as almost all the ls_err's are
 *   fatal and instantly break out to main with no subsequent system or 
 *   library calls that would reset errno.
 */
void ls_perror(ls_err err, char *pname) {
    char str_buf[4096];
    int err = errno;
    memcpy(str_buf, pname, strlen(pname) + 1);
    memcpy(str_buf + strlen(pname), ls_err_msg[err], \
        strlen(ls_err_msg[err]) + 1);
    if (err) {
        errno = err;
        perror(str_buf);
    }
    else {
        fprintf(stderr, "%s: %s", pname, str_buf);
    }
}