#ifndef __LS_H
#define __LS_H
#include <dirent.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include "bst.h"

#define OPT_STRING "a"

#define OPT_a_MASK 0x1

int get_flags(int, char**);
int get_dir_listings(DIR*, tree*, int);
int _add_entry(tree*, char*);

#endif /* __LS_H */