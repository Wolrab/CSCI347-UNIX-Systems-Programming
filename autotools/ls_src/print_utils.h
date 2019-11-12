#ifndef __PRINT_UTILS_H
#define __PRINT_UTILS_H
#include <string.h>
#include <limits.h>
#include <assert.h>
#include <stdlib.h>

#define MAX_VAL_4BYTES_S 2147483647
#define MAX_VAL_4BYTES_U 4294967295
#define MAX_VAL_8BYTES_S 9223372036854775807LL
#define MAX_VAL_8BYTES_U 18446744073709551615ULL

// Constants for get_f_max_strlen
extern const int max_strlen_4bytes_s;
extern const int max_strlen_4bytes_u;
extern const int max_strlen_8bytes_s;
extern const int max_strlen_8bytes_u;

// Type formatting defines
#define INO_PRINTF "%llu"
#define NLINK_PRINTF "%d"
#define OFF_PRINTF "%lld"
#define UID_PRINTF "%d"
#define GID_PRINTF "%d"

// Gets the maximum amount of characters needed to represent the given format
//   string.
int get_f_max_strlen(char *format);

#endif /* __PRINT_UTILS_H */
