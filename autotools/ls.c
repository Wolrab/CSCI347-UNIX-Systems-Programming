#include "ls.h"

// Quick macro to help in navigating argv
#define IS_OPT(s) (s[0]=='-')

// A reasonable buffer for reasonable strings
char buffer[4096];
const int buffer_size = 4096;

/* Parses options, and in the spirit of the real ls parses all non-option
 *   arguments as paths to sequentially evaluate. The sequence of paths is 
 *   evaluated with FIFO order.
 * Returns: 0 on success, 1 on error.
 */
int main(int argc, char **argv) {
    char options;
    unsigned int path_ind;
    bool cont;
    int err, ret = 0;

    options = get_options(argc, argv);
    if (options == -1) {
        printf("Usage: ls [-a] [file...]\n");
        return 1;
    }

    ls_err_prog = argv[0];

    for (path_ind = 1; path_ind<argc && IS_OPT(argv[path_ind]); path_ind++) continue;
    if (path_ind == argc) {
        ret = _ls(".", options);
    }
    else if (path_ind == argc-1) {
        ret = _ls(argv[path_ind], options);
        if (ret && ls_err_state != LS_ERR_NONE) {
            ls_perror();
        }
    }
    else {
        do {
            printf("%s:\n", argv[path_ind]);

            err = _ls(argv[path_ind], options);
            if (err) {
                ret |= err;
                if (ls_err_state != LS_ERR_NONE) {    
                    ls_perror();
                    if (ls_err_state == LS_ERR_MALLOC) break;
                    ls_err_state = LS_ERR_NONE;
                    ls_err_path = NULL;
                }
            }
            path_ind++;
            cont = path_ind < argc;
            
            if (cont) printf("\n");
        } while (cont);
    }

    return ret;
}

/* For printing ls errors with meaningfull errno values. Prints the program name, the error 
 *   message indexed by ls_error_state, and passes that string to perror to be handled by
 *   perror
 * It should be noted, but I am not checking the returns of *printf's in here. If the output 
 *   for error checking fails, then god has willed the program to crash, and he wishes his 
 *   reasons to remain an eternal secret. I shall respect those wishes.
 */
void ls_perror() {
    int err = errno;
    if (ls_err_state == LS_ERR_MALLOC) {
        sprintf(buffer, "%s: %s", ls_err_prog, ls_err_str[ls_err_state]);
    }
    else if (ls_err_state == LS_ERR_DIR_ACC || ls_err_state == LS_ERR_DIR_READ_ENTRY \
             || ls_err_state == LS_ERR_LSTAT || ls_err_state == LS_ERR_LINK_OPEN) {
        sprintf(buffer, "%s: ", ls_err_prog);
        sprintf(buffer + strlen(buffer), ls_err_str[ls_err_state], ls_err_path);
    }
    errno = err;
    perror(buffer);
}

/* Prints the entries of a single path to stdout with a given set of options
 * Returns: 0 on success, 1 on error
 */
int _ls(char *path, char options) {
    DIR *d;
    f_list dir_entries;
    int ret, err;

    errno = 0;
    d = opendir(path);
    if (d == NULL) {
        ls_err_state = LS_ERR_DIR_ACC;
        ls_err_path = path;
        return 1;
    }

    if (f_list_init(&dir_entries) == NULL) {
        ls_err_state = LS_ERR_MALLOC;
        return 1;
    }

    if (options & opt_l_mask)
        ret = get_ent_stats(d, &dir_entries, path, options);
    else 
        ret = get_ent_names(d, &dir_entries, path, options);

    if (ret) {
        switch (ls_err_state) {
        case LS_ERR_NONE:
            break;
        case LS_ERR_MALLOC:
            goto err_list_cleanup;
        case LS_ERR_DIR_READ_ENTRY:
            goto err_list_cleanup;
        case LS_ERR_LSTAT:
            goto err_list_cleanup;
        }
        switch (dir_entries.err) {
        case FL_ERR_NONE:
            break;
        case FL_ERR_MALLOC:
            ls_err_state = LS_ERR_MALLOC;
            goto err_list_cleanup;
        case FL_ERR_ARRAY_OVERFLOW:
            fprintf(stderr, "%s\n", f_list_err_out[FL_ERR_ARRAY_OVERFLOW]);
            fprintf(stderr, "printing entries available\n");
            break;
        }
    }
   
    closedir(d);

    f_list_sort(&dir_entries);
    if (options & opt_l_mask) {
        if (output_ent_stats(&dir_entries)) {
            // None all are handled within the function
        }
    }
    else
        output_ent_names(&dir_entries);
    
    f_list_delete_data(&dir_entries);
    return ret;

    err_list_cleanup:
    err = errno;
    f_list_delete_data(&dir_entries);
    errno = err;
    return ret;
}

/* Gets flag bits and pushes non-options to the end of argv.
 * Each character in a string that starts with a - is considered an option.
 * Returns: 0 on success, -1 if an unknown option is found.
 */
char get_options(int argc, char **argv) {
    int options = 0;
    char opt = getopt(argc, argv, opt_string);

    while (opt != -1) {
        switch (opt) {
        case 'a':
            options |= opt_a_mask;
            break;
        case 'l':
            options |= opt_l_mask;
            break;
        case '?':
            return -1;
        }
        opt = getopt(argc, argv, opt_string);
    }
    return options;
}

/* Gets the names of all the files in directory stream d and places
 *   them in dir_entries
 * Returns: 0 if successful, 1 on error
 */
int get_ent_names(DIR *d, f_list *dir_entries, const char *path, const char options) {
    struct dirent *ent;

    errno = 0;
    ent = readdir(d);
    while (ent != NULL) {
        if (ent->d_name[0] == '.' && !(options & opt_a_mask)) {
            errno = 0;
            ent = readdir(d);
            continue;
        }

        if (f_list_add_elem(dir_entries, ent->d_name, NULL, NULL)) {
            if (dir_entries->err != FL_ERR_NONE)
                return 1;
        }

        errno = 0;
        ent = readdir(d);
    }
    if (errno != 0) {
        ls_err_state = LS_ERR_DIR_READ_ENTRY;
        ls_err_path = path;
        return 1;
    }
    return 0;
}

/* Gets the names and stat structures of all the files in the directory stream
 *   and places them in dir_entries
 * Returns: 0 if successful, 1 on error
 */
int get_ent_stats(DIR *d, f_list *dir_entries, const char *path, const char options) {
    struct dirent *ent;
    struct stat *ent_stat;
    char path_buf[PATH_MAX], *link_buf;
    int path_len = strlen(path);

    memcpy(path_buf, path, path_len + 1);
    if (path_buf[path_len-1] != '/') {
        path_buf[path_len] = '/';
        path_len++;
    }

    errno = 0;
    ent = readdir(d);
    while (ent != NULL) {
        if (ent->d_name[0] == '.' && !(options & opt_a_mask)) {
            errno = 0;
            ent = readdir(d);
            continue;
        }
        
        errno = 0;
        ent_stat = malloc(sizeof(struct stat));
        if (ent_stat == NULL) {
            ls_err_state = LS_ERR_MALLOC;
            return 1;
        }

        memcpy(path_buf + path_len, ent->d_name, strlen(ent->d_name) + 1);
        errno = 0;
        if (lstat(path_buf, ent_stat) < 0) {
            free(ent_stat);
            ls_err_state = LS_ERR_LSTAT;
            ls_err_path = path_buf;
            return 1;
        }

        if (S_ISLNK(ent_stat->st_mode)) {
            errno = 0;
            link_buf = malloc(ent_stat->st_size);
            if (link_buf == NULL && errno) {
                free(ent_stat);
                ls_err_state = LS_ERR_MALLOC;
                return 1;
            }
            if (readlink(path_buf, link_buf, PATH_MAX) < 0) {
                free(ent_stat);
                ls_err_state = LS_ERR_LINK_OPEN;
                ls_err_path = path_buf;
                return 1;
            }
            if (f_list_add_elem(dir_entries, ent->d_name, ent_stat, link_buf)) {
                if (dir_entries->err != FL_ERR_NONE)
                    return 1;
            }
        }
        else {
            if (f_list_add_elem(dir_entries, ent->d_name, ent_stat, NULL)) {
                if (dir_entries->err != FL_ERR_NONE)
                    return 1;
            }
        }

        errno = 0;
        ent = readdir(d);
    }
    if (errno != 0) {
        ls_err_state = LS_ERR_DIR_READ_ENTRY;
        ls_err_path = path;
        return 1;
    }
    return 0;
}

void output_ent_names(f_list *dir_entries) {
    int i;
    for (i = 0; i < dir_entries->len; i++) {
        printf("%s\n", dir_entries->f_data[i]->f_name);
    }
}

int output_ent_stats(f_list *dir_entries) {
    struct stat_out_s *stat_strings;
    int i, j, ret = 0;
    unsigned int max_char_usr = 0, max_char_grp = 0, max_link_dig = 0, max_size_dig = 0;

    errno = 0;
    stat_strings = malloc(sizeof(struct stat_out_s) * (dir_entries->len));
    if (stat_strings == NULL && errno) {
        ls_err_state = LS_ERR_MALLOC;
        return 1;
    }

    for (i = 0; i < dir_entries->len; i++) {
        if (get_stat_out(stat_strings + i, dir_entries->f_data[i]->f_stat)) {
            if (ls_err_state == LS_ERR_MALLOC)
                goto cleanup_inloop_stat_strings;
        }
        ret = strlen(stat_strings[i].usr);
        if (ret > max_char_usr) max_char_usr = ret;
        ret = strlen(stat_strings[i].grp);
        if (ret > max_char_grp) max_char_grp = ret;
        ret = get_digits(dir_entries->f_data[i]->f_stat->st_nlink);
        if (ret > max_link_dig) max_link_dig = ret;
        ret = get_digits(dir_entries->f_data[i]->f_stat->st_size);
        if (ret > max_size_dig) max_size_dig = ret;
    }
    
    for (i = 0; i < dir_entries->len; i++) {
        if (dir_entries->f_data[i]->link_path == NULL)
            printf("%s %*lu %*s %*s %*ld %s %s\n", stat_strings[i].mode, max_link_dig, dir_entries->f_data[i]->f_stat->st_nlink, \
                max_char_usr, stat_strings[i].usr, max_char_grp, stat_strings[i].grp, max_size_dig, dir_entries->f_data[i]->f_stat->st_size, \
                stat_strings[i].mtim, dir_entries->f_data[i]->f_name);
        else
            printf("%s %*lu %*s %*s %*ld %s %s -> %s\n", stat_strings[i].mode, max_link_dig, dir_entries->f_data[i]->f_stat->st_nlink, \
                    max_char_usr, stat_strings[i].usr, max_char_grp, stat_strings[i].grp, max_size_dig, dir_entries->f_data[i]->f_stat->st_size, \
                    stat_strings[i].mtim, dir_entries->f_data[i]->f_name, dir_entries->f_data[i]->link_path);
    }
    
    for (i = 0; i < dir_entries->len; i++) {
        free(stat_strings[i].usr);
        free(stat_strings[i].grp);
    }
    free(stat_strings);
    return 0;

    cleanup_inloop_stat_strings:
    for (j = 0; j < i; j++) {
        free(stat_strings[j].usr);
        free(stat_strings[i].grp);
    }
    free(stat_strings);
    return 1;
}

int get_stat_out(struct stat_out_s *stat_out, struct stat *f_stat) {
    get_mode(stat_out->mode, f_stat->st_mode);
    if (get_usr(&(stat_out->usr), f_stat->st_uid))
        return 1;
    if (get_grp(&(stat_out->grp), f_stat->st_gid))
        goto cleanup_usr;
    get_mtim(&(stat_out->mtim), &(f_stat->st_mtime));
    return 0;

    cleanup_usr:
    free(stat_out->usr);
    return 1;
}

void get_mode(char *mode_s, mode_t mode) {
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

int get_usr(char **usr, uid_t uid) {
    struct passwd *passwd_ent;
    errno = 0;
    passwd_ent = getpwuid(uid);
    if (errno) {
        perror("getpwuid");
    }
    if (passwd_ent == NULL) {
        errno = 0;
        *usr = malloc(11);
        if (*usr == NULL) {
            ls_err_state = LS_ERR_MALLOC;
            return 1;
        }
        sprintf(*usr, "%u", uid);
    }
    else {
        errno = 0;
        *usr = malloc(strlen(passwd_ent->pw_name) + 1);
        if (*usr == NULL && errno) {
            ls_err_state = LS_ERR_MALLOC;
            return 1;
        }
        memcpy(*usr, passwd_ent->pw_name, strlen(passwd_ent->pw_name) + 1);
    }
    return 0;
}

int get_grp(char **grp, uid_t gid) {
    struct group *group_ent;
    errno = 0;
    group_ent = getgrgid(gid);
    if (errno) {
        perror("getgrgid");
    }
    if (group_ent == NULL) {
        errno = 0;
        *grp = malloc(11);
        if (*grp == NULL) {
            ls_err_state = LS_ERR_MALLOC;
            return 1;
        }
        sprintf(*grp, "%u", gid);
    }
    else {
        *grp = malloc(strlen(group_ent->gr_name) + 1);
        if (*grp == NULL && errno) {
            ls_err_state = LS_ERR_MALLOC;
            return 1;
        }
        memcpy(*grp, group_ent->gr_name, strlen(group_ent->gr_name) + 1);
    }
    return 0;
}

int get_mtim(char **mtim_s, time_t *mtim) {
    struct tm *t = localtime(mtim);
    errno = 0;
    if (strftime(buffer, buffer_size, "%b %e %H:%M %Y", t) == 0 && errno) {
        memset(buffer, '?', 17);
        buffer[17] = '\0';
    }
    *mtim_s = malloc(strlen(buffer) + 1);
    memcpy(*mtim_s, buffer, strlen(buffer) + 1);
    return 0;
}

unsigned int get_digits(unsigned long n) {
    unsigned int dig = 1;
    unsigned long place = 10;
    while (n % place != n) {
        dig++;
        if (place * 10 < place)
            break;
        place *= 10;
    }
    return dig;
}
