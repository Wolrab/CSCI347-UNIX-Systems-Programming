#ifndef __LS_H
#define __LS_H
#include <dirent.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include "bst.h"

#define OPT_STRING "a"

#define OPT_a_MASK 0x1

#define OPENDIR_ERROR_F "opendir: can't open '%s'"
#define OPENDIR_ERROR_NONF_LEN strlen(OPENDIR_ERROR_F)-2

void path_error_out(char *path);
int get_flags(int argc, char **argv);
int get_dir_listings(DIR *d, tree *list, int flags);
// Creates a dynamic copy of 
int _add_entry(tree *list, char *name);

#endif /* __LS_H */