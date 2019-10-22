#include "ls.h"

// Define external constants
const char *const ls_err_str[] = {
    "no error%s",
    "malloc%s",
    "cannot open dir '%s'",
    "cannot read an entry in directory '%s'",
    "cannot stat '%s'",
    "duplicate file '%s' found: program data has most likely been corrupted"
};
const bool ls_err_errno[] = {false, true, true, true, true, false};

/** 
 * The ls program. Prints all entries of current directory to stdout and exits.
 *   Output style is modified by options passed into the program.
 * After a call to ls, if any error occurs, the errno value is preserved and 
 *   the program unwinds all the way back to main. In that respect a lot of 
 *   the globals are simply QOL enhancements, but they are so tied into the 
 *   whole system there's really no going back. This program flow must be 
 *   maintained for safety and to not feed ls_perror garbage.
 * The return value is basically a flag alerting the calling function a global
 *   error value has been set.
 * Returns: 0 on success, 1 on error.
 */
int main(int argc, char **argv) {
    int ret = 0;

    ret = get_options(argc, argv);
    if (ret) {
        printf("Usage: ls [-%s]\n", OPTION_STRING);
        return ret;
    }

    clear_ls_err();

    ret = ls(".");
    if (ret) {
        ls_perror(argv[0]);
        clear_ls_err();
    }

    return ret;
}

/**
 * Prints the entries of the directory specified by path to stdout.
 * Returns: 0 on success, 1 on error and sets ls_err_state appropriately.
 */
int ls(char *path) {
    list dir_entries;
    int ret = 0;

    if (list_init(&dir_entries) == NULL) {
        set_ls_err(LS_ERR_MALLOC, NULL);
        return 1;
    }

    ret = get_entries(path, &dir_entries);
    if (ret) {
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
 * Opens a directory entry and puts all values from it into dir_entries through
 *   a call to read_dir_stream.
 * Returns: 0 if successful, 1 on error and sets ls_err_state appropriately.
 */
int get_entries(const char *path, list *dir_entries) {
    DIR *d = NULL;
    int ret = 0;

    errno = 0;
    d = opendir(path);
    if (d == NULL) {
        set_ls_err(LS_ERR_DIR_OPEN, path);
        ret = 1;
    }
    else {
        ret = read_dir_stream(d, path, dir_entries);
    }

    closedir(d);
    return ret;
}

/**
 * Iterates through the directory stream d, filling dir_entries with each entry.
 * Returns: 0 if successful, 1 on error and sets ls_err_state appropriately.
 */
int read_dir_stream(DIR *d, const char *path, list *dir_entries) {
    struct dirent *ent = NULL;
    struct stat *ent_stat = NULL;
    list_err err = LIST_ERR_NONE;
    int ret = 0;

    errno = 0;
    ent = readdir(d);
    while (ent != NULL) {
        if (ent->d_name[0] == '.' && !(option_a)) {
            errno = 0;
            ent = readdir(d);
            continue;
        }

        if (option_l) {
            ent_stat = get_stat(ent->d_name, path);
            if (ent_stat == NULL) {
                goto cleanup_mid_loop;
            }
        }

        err = list_add_ordered(dir_entries, ent->d_name, ent_stat);
        switch (err) {
        case LIST_ERR_NONE:
            break;
        case LIST_ERR_MALLOC:
            set_ls_err(LS_ERR_MALLOC, NULL);
            goto cleanup_mid_loop;
        case LIST_ERR_DUP_ENTRY:
            set_ls_err(LS_ERR_DUP_ENTRY, ent->d_name);
            goto cleanup_mid_loop;
        }
        
        errno = 0;
        ent = readdir(d);
    }
    if (ent == NULL && errno != 0) {
        set_ls_err(LS_ERR_DIR_READ_ENTRY, path);
        ret = 1;
    }
    return ret;

    cleanup_mid_loop:
    if (ent_stat != NULL) {
        free(ent_stat);
    }
    return 1;
}

/**
 * Concatenates name and path and finds the new path's associated
 *   stat structure.
 * Returns: The stat structure if successful and NULL on error, 
 *   setting ls_err_state appropriately.
 */
struct stat* get_stat(const char *f_name, const char *path) {
    struct stat *f_stat = NULL;
    
    errno = 0;
    f_stat = malloc(sizeof(struct stat));
    if (f_stat == NULL) {
        set_ls_err(LS_ERR_MALLOC, NULL);
        return NULL;
    }

    memcpy(path_buffer, path, strlen(path) + 1);
    if (path_buffer[strlen(path)-1] != '/') {
        path_buffer[strlen(path)+1] = '\0';
        path_buffer[strlen(path)] = '/';
    }
    memcpy(path_buffer + strlen(path_buffer), f_name, strlen(f_name) + 1);

    errno = 0;
    if (stat(path_buffer, f_stat) < 0) {
        set_ls_err(LS_ERR_STAT, path_buffer);

        free(f_stat);
        f_stat = NULL;
    }

    return f_stat;
}

/**
 * Outputs the filenames stored in dir_entries to stdout
 */
void output_ent_names(list *dir_entries) {
    node *dir_entries_i;

    dir_entries_i = *dir_entries;
    while (dir_entries_i->next != NULL) {
        printf("%s\n", dir_entries_i->data.f_name);
        dir_entries_i = dir_entries_i->next;
    }
}

/**
 * Outputs the filenames and stat information in dir_entries to stdout
 * Returns: 0 if successful, 1 on error and sets ls_err_state appropriately.
 */
int output_ent_stats(list *dir_entries) {
    struct stat_out_s stat_out;
    node *dir_entries_i;
    int ret = 0;
    unsigned long nlink_cast;
    long size_cast;
    
    dir_entries_i = *dir_entries;
    while (dir_entries_i->next != NULL) {
        ret = fill_stat_out(&stat_out, &(dir_entries_i->data));
        if (ret) {
            return ret;
        }

        nlink_cast = (unsigned long)dir_entries_i->data.f_stat->st_nlink;
        size_cast = (long)dir_entries_i->data.f_stat->st_size;

        printf("%s %lu %s %s %ld %s %s\n", stat_out.mode_str, 
            nlink_cast, stat_out.usr_str, stat_out.grp_str, 
            size_cast, stat_out.mtim_str, stat_out.f_name);

        free_stat_out(&stat_out);
        dir_entries_i = dir_entries_i->next;
    }
    return ret;
}

/**
 * Fills stat_out using the fields given in data.
 * Returns: 0 if successful, 1 on error and sets ls_err_state appropriately.
 */
int fill_stat_out(struct stat_out_s *stat_out, struct data_s *data) {
    int ret = 0;

    get_mode_str(stat_out->mode_str, data->f_stat->st_mode);

    ret = get_usr_str(&(stat_out->usr_str), data->f_stat->st_uid);
    if (ret) {
        goto exit;
    }

    ret = get_grp_str(&(stat_out->grp_str), data->f_stat->st_gid);
    if (ret) {
        goto cleanup_usr;
    }
    
    ret = get_mtim_str(&(stat_out->mtim_str), data->f_stat->st_mtime);
    if (ret) {
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
 * Fills a fixed sized character sequence representing a file's mode.
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
 * Finds a user name given a uid. If the user name cannot be found for some
 *   reason, the value of uid is stored in usr instead.
 * Returns: 0 if successful, 1 on error and sets ls_err_state appropriately.
 */
int get_usr_str(char **usr_str, uid_t uid) {
    struct passwd *passwd_ent;

    passwd_ent = getpwuid(uid);
    if (passwd_ent == NULL) {
        sprintf(str_buffer, "%u", uid);

        errno = 0;
        *usr_str = malloc(strlen(str_buffer));
        if (*usr_str == NULL) {
            set_ls_err(LS_ERR_MALLOC, NULL);
            return 1;
        }

        sprintf(*usr_str, "%s\n", str_buffer);
    }
    else {
        errno = 0;
        *usr_str = malloc(strlen(passwd_ent->pw_name) + 1);
        if (*usr_str == NULL && errno) {
            set_ls_err(LS_ERR_MALLOC, NULL);
            return 1;
        }

        memcpy(*usr_str, passwd_ent->pw_name, strlen(passwd_ent->pw_name) + 1);
    }
    return 0;
}

/**
 * Finds a group name given a gid. If the group name cannot be found, the value
 *   of gid is stored in grp instead.
 * Returns: 0 if successful, 1 on error and sets ls_err_state appropriately.
 */
int get_grp_str(char **grp_str, gid_t gid) {
    struct group *group_ent;

    group_ent = getgrgid(gid);
    if (group_ent == NULL) {
        sprintf(str_buffer, "%u", gid);

        errno = 0;
        *grp_str = malloc(strlen(str_buffer));
        if (*grp_str == NULL) {
            set_ls_err(LS_ERR_MALLOC, NULL);
            return 1;
        }

        sprintf(*grp_str, "%s\n", str_buffer);
    }
    else {
        errno = 0;
        *grp_str = malloc(strlen(group_ent->gr_name) + 1);
        if (*grp_str == NULL && errno) {
            set_ls_err(LS_ERR_MALLOC, NULL);
            return 1;
        }

        memcpy(*grp_str, group_ent->gr_name, strlen(group_ent->gr_name) + 1);
    }
    return 0;
}

/** 
 * Gets the formatted date from mtim.
 * Returns: 0 if successful, 1 on error and sets ls_err_state appropriately.
 */
int get_mtim_str(char **mtim_str, time_t mtim) {
    struct tm *t;
    
    t = localtime(&mtim);
    errno = 0;
    if (strftime(str_buffer, str_buffer_size, "%b %e %H:%M", t) == 0 && errno) {
        memset(str_buffer, '?', 12);
        str_buffer[12] = '\0';
    }
    errno = 0;
    *mtim_str = malloc(strlen(str_buffer) + 1);
    if (*mtim_str == NULL && errno) {
        set_ls_err(LS_ERR_MALLOC, NULL);
        return 1;
    }
    memcpy(*mtim_str, str_buffer, strlen(str_buffer) + 1);
    return 0;
}

/**
 * Frees all dynamically allocated fields of stat_out
 */
void free_stat_out(struct stat_out_s *stat_out) {
    free(stat_out->usr_str);
    free(stat_out->grp_str);
    free(stat_out->mtim_str);
}

/**
 * Checks argv for options and sets option flags.
 * Returns: 0 on success, 1 if an unknown option is found.
 */
int get_options(const int argc, char **argv) {
    char opt = -1;
    
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
            return 1;
        }
        opt = getopt(argc, argv, OPTION_STRING);
    }
    return 0;
}

/**
 * Prints ls errors. Prints error for errno value if it makes sense for
 *   the given ls_err.
 */
void ls_perror(char *program) {
    int err;

    err = errno;
    sprintf(str_buffer, "%s: ", program);
    sprintf(str_buffer + strlen(str_buffer), ls_err_str[ls_err_state], \
            ls_err_path);

    errno = err;
    if (ls_err_errno[ls_err_state]) {
        perror(str_buffer);
    }
    else {
        fprintf(stderr, "%s\n", str_buffer);
    }
}

/**
 * Sets the current ls_err_state and path (NULL if path not needed)
 */
void set_ls_err(ls_err err, const char *path) {
    ls_err_state = err;
    if (path != NULL) {
        memcpy(ls_err_path, path, strlen(path) + 1);
    }
}

/**
 * Clears the current ls_err_state and path
 */
void clear_ls_err() {
    ls_err_state = LS_ERR_NONE;
    memset(ls_err_path, '\0', ls_err_path_size);
}