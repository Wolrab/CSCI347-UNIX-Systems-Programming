#include "ls.h"

// Quick macro to help in navigating argv
#define IS_OPT(s) (s[0]=='-')

// Arbitrary whatever buffer (DANGER DANGER HIGH VOLTAGE)
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
             || ls_err_state == LS_ERR_LSTAT || ls_err_state == LS_ERR_FOPEN \
             || ls_err_state == LS_ERR_GETLINE) {
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
        case LS_ERR_GETLINE:
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

        if (f_list_add_elem(dir_entries, ent->d_name, NULL)) {
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
    char path_buf[PATH_MAX];
    int path_len = strlen(path), err;

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
            err = errno;
            free(ent_stat);
            errno = err;
            ls_err_state = LS_ERR_LSTAT;
            ls_err_path = path_buf; // File path
            return 1;
        }

        if (f_list_add_elem(dir_entries, ent->d_name, ent_stat)) {
            if (dir_entries->err != FL_ERR_NONE)
                return 1;
        }

        errno = 0;
        ent = readdir(d);
    }
    if (errno != 0) {
        ls_err_state = LS_ERR_DIR_READ_ENTRY;
        ls_err_path = path; // Directory path
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
    int i;
    FILE *passwd, *group;

    errno = 0;
    stat_strings = malloc(sizeof(struct stat_out_s) * (dir_entries->len-1));
    if (stat_strings == NULL && errno) {
        ls_err_state = LS_ERR_MALLOC;
        return 1;
    }
    errno = 0;
    passwd = fopen("/etc/passwd", "r");
    if (passwd == NULL) {
        ls_err_state = LS_ERR_FOPEN;
        ls_err_path = "/etc/passwd";
        ls_perror();
        ls_err_state = LS_ERR_NONE;
        ls_err_path = NULL;
    }
    errno = 0;
    group = fopen("/etc/group", "r");
    if (group == NULL) {
        ls_err_state = LS_ERR_FOPEN;
        ls_err_path = "/etc/group";
        ls_perror();
        ls_err_state = LS_ERR_NONE;
        ls_err_path = NULL;
    }

    for (i = 0; i < dir_entries->len; i++) {
        if (get_stat_out(&stat_strings[i], dir_entries->f_data[i]->f_stat, passwd, group)) {
            if (ls_err_state == LS_ERR_MALLOC)
                goto cleanup_group;
            if (ls_err_state == LS_ERR_GETLINE)
                goto cleanup_group;
            if (ls_err_state == LS_ERR_REGEX) {
                fprintf(stderr, "Could not print stats for '%s', continuing to next element\n", dir_entries->f_data[i]->f_name);
                ls_err_state = LS_ERR_NONE;
            }
        }
    }
    // BIG TODO WHY BREAK
    //if (group != NULL) fclose(group);
    //if (passwd != NULL) fclose(passwd);

    // janky test print pray for me
    for (i = 0; i < dir_entries->len; i++) {
        printf("%s %s %s %s\n", stat_strings[i].mode, stat_strings[i].nlink, stat_strings[i].usr, stat_strings[i].size);
    }
    /* Obviously I don't know how to file, and am fucking up so hard that trying to close the files creates undefined behavior
-rw-rw-r-- 1 dingus: 517
-rw-rw-r-- 1 1000 4720
-rw-rw-r-- 1  2891
-rwxrwxr-x 1  35960
-rw-rw-r-- 1  13384
-rw-rw-r-- 1  3491
-rw-rw-r-- 1  195
-rw-r--r-- 1  542
drwxrwxr-x 2  4096
drwxr-xr-x 2  4096
    */


    // 1. Find the largest string for each non-constant element
    // 2. Print them out in a cool formated manner!
    free(stat_strings);

    return 0;

    cleanup_group:
    fclose(group);
    cleanup_passwd:
    fclose(passwd);
    cleanup_stat_strings:
    free(stat_strings);
    return 1;
}


int get_stat_out(struct stat_out_s *stat_out, struct stat *f_stat, FILE *passwd, FILE *group) {
    get_mode(stat_out->mode, f_stat->st_mode);
    get_nlink(stat_out->nlink, f_stat->st_nlink);
    if (get_usr(&(stat_out->usr), f_stat->st_uid, f_stat->st_gid, passwd))
        return 1;
    //if (get_grp(&(stat_out->grp), f_stat->st_gid, group))
    //    goto cleanup_usr;
    get_size(stat_out->size, f_stat->st_size);
    //if (get_mtim(&(stat_out->mtim), f_stat->st_mtime))
    //    goto cleanup_grp;
    return 0;

    cleanup_grp:
    free(stat_out->grp); // Correct: reference to line from getline which is allocated on the heap
    cleanup_usr:
    free(stat_out->usr); // Correct: reference to line from getline which is allocated on the heap
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

void get_nlink(char *nlink_s, nlink_t nlink) {
    sprintf(buffer, "%lu", nlink);
    memcpy(nlink_s, buffer, strlen(buffer) + 1);
}

int get_usr(char **uid_s, uid_t uid, uid_t gid, FILE *passwd) {
    char pat[34+10+10+1]; // pattern, maxmimum digits for unsigned integers and null character
    regex_t reg;
    regmatch_t match[2];
    char *line = NULL;
    int err, ret, reg_ret;
    size_t len;

    if (passwd == NULL) {
        *uid_s = malloc(11);
        sprintf(buffer, "%u", uid);
        memcpy(*uid_s, buffer, strlen(buffer) + 1);
    }

    else {
        // TODO more errors jesus
        lseek(passwd, 0, SEEK_SET);
        sprintf(pat, "^\\([[:print:]]*\\):[[:print:]]*:%u:%u", uid, gid);
        reg_ret = regcomp(&reg, pat, 0);
        if (reg_ret) {
            regerror(reg_ret, &reg, buffer, buffer_size);
            fprintf(stderr, ls_err_str[LS_ERR_REGEX], buffer);
            ls_err_state = LS_ERR_REGEX;
            return 1;
        }

        errno = 0;
        ret = getline(&line, &len, passwd);
        while ( !(ret < 0) ) {
            reg_ret = regexec(&reg, line, 2, match, 0);
            if (reg_ret != REG_NOMATCH) break;

            free(line);
            line = NULL;
            errno = 0;
            ret = getline(&line, &len, passwd);
        }
        if (ret < 0 && errno) {
            err = errno;
            free(line);
            errno = err;
            ls_err_state = LS_ERR_GETLINE;
            ls_err_path = "/etc/passwd";
            return 1;
        }
        else if (ret < 0 && reg_ret == REG_NOMATCH) {
            *uid_s = malloc(11);
            sprintf(buffer, "%u", uid);
            memcpy(*uid_s, buffer, strlen(buffer) + 1);
        }
        else if (reg_ret != REG_NOMATCH) {
            *uid_s = line + match[1].rm_so;
            line[match[1].rm_eo] = '\0';
        }
    }
    return 0;
}

int get_grp(char **gid_s, uid_t gid, FILE *group) {

}

void get_size(char *size_s, off_t size) {
    sprintf(buffer, "%ld", size);
    memcpy(size_s, buffer, strlen(buffer) + 1);

}

// Jesus I still gotta do this
int get_mtim(char **mtim_s, struct timespec *mtim) {
    time_t time = (time_t)mtim->tv_sec;
    
}
