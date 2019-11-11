#ifndef __PRINT_UTILS_H
#define __PRINT_UTILS_H
#include <string.h>
#include <limits.h>
#include <assert.h>

// Type formatting defines
#define INO_PRINTF "%lu"
#define NLINK_PRINTF "%d"
#define OFF_PRINTF "%lld"
#define UID_PRINTF "%d"
#define GID_PRINTF "%d"

int get_f_max_strlen(char *format);

#endif /* __PRINT_UTILS_H */