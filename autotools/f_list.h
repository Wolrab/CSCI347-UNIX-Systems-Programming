#ifndef __F_LIST_H
#define __F_LIST_H
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <errno.h>
#include <assert.h>

// Logarithmicaly increasing and kind of arbitrary array sizes to itterate through
static const unsigned _f_list_size[] = {
    0x0000000F, 0x000000FF, 0x000003FF, 0x00000FFF, \
    0x00003FFF, 0x0000FFFF, 0x0003FFFF, 0x000FFFFF, \
    0x003FFFFF, 0x00FFFFFF, 0x03FFFFFF, 0x0FFFFFFF, \
    0x3FFFFFFF, 0x7FFFFFFF // Maximum size to be indexed by a signed int because I'm lazy and sort uses the signed bit for some (safe) termination cases
};
static const unsigned _f_list_size_num = 14;

typedef enum f_list_err f_list_err;
enum f_list_err {
    FL_ERR_NONE,
    FL_ERR_MALLOC,          // errno: meaningful
    FL_ERR_ARRAY_OVERFLOW   // errno: not meaninful
};
static const char *const f_list_err_out[] = {
    "no error",
    "malloc",
    "array overflow: exceeded maximum array size"
};

// Needed to have proper order between each f_name
struct key_s {
    char *cmp_char; // The string set to all lowercase, all non-character comparisons relative to locale are held
    char *cmp_case; // The string itself, for the case that all characters match and casewise comparison is required
};

// All the necessary file information
struct f_data_s {
    char f_name[NAME_MAX]; // Ensures memory is managed appropriately, no finickey bs
    struct key_s key;
    struct stat *f_stat;
    char *link_path;
};

typedef struct f_list_s f_list;
// f_data plus information for the managment of the array
struct f_list_s {
    struct f_data_s **f_data;
    int len;
    int size_i;
    f_list_err err;
};

// Small lemma for the quicksort algorithm.
#define SWAP(x, y, temp) {temp = x; x = y; y = temp;}
// Used to determine sorting order, this should only called internally.
int key_order(struct key_s *k1, struct key_s *k2);
// Creates a key for each given file name, this should only be called internally.
int make_key(char *f_name, struct key_s *key, f_list_err *err);

// Always use to initialize a new f_list.
f_list* f_list_init(f_list *fl);
// Always use to initialize a new f_list if you care about the seed (for quicksort obvi)
f_list* f_list_init_seed(f_list *fl, unsigned int seed);
// Copies values of both f_name and f_stat!
int f_list_add_elem(f_list *fl, char *f_name, struct stat *f_stat, char *link_path);
// Called by f_list_add_elem, the f_list_size array determines each increment in the 
//   size of the array each time it's resized.
int _f_list_resize(f_list* fl);
// User function for an in-place sort of an f_list.
void f_list_sort(f_list* fl);
// Quicksort implementation, do not call.
void _f_list_sort_r(f_list* fl, int p, int r);
// Partition function for quicksort, do not call.
int _f_list_partition(f_list* fl, int p, int r);
// Deletes the f_list
void f_list_delete_data(f_list* fl);

#endif /* __F_LIST_H */