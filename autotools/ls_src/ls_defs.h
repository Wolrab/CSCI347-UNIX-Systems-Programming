#ifndef __LS_DEFS_H
#define __LS_DEFS_H
#include <stdbool.h>

// 
#define INO_PRINTF "%lu"
#define NLINK_PRINTF "%d"
#define OFF_PRINTF "%lld"
#define UID_PRINTF "%d"
#define GID_PRINTF "%d"

// Option flags
extern bool option_a;
extern bool option_l;
extern bool option_i;
extern bool option_d;

#endif /* __LS_DEFS_H */