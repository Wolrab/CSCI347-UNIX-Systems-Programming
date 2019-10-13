#ifndef __DIR_ARR_H
#define __DIR_ARR_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <errno.h>

// Logarithmicaly increasing and kind of arbitrary array sizes to itterate through
static const unsigned _f_list_size[] = {0x0000000F, 0x000000FF, 0x000003FF, 0x00000FFF, \
                                        0x00003FFF, 0x0000FFFF, 0x0003FFFF, 0x000FFFFF, \
                                        0x003FFFFF, 0x00FFFFFF, 0x03FFFFFF, 0x0FFFFFFF, \
                                        0x3FFFFFFF, 0xFFFFFFFF};
#define _F_LIST_SIZE_LEN 14

// Needed to have proper order between each f_name
struct key_s {
    char *cmp_char; // The string set to all lowercase, all non-character comparisons relative to locale are held
    char *cmp_case; // The string itself, for the case that all characters match and casewise comparison is required
};

// All the necessary file information
struct f_data_s {
    char *f_name;
    struct key_s key;
    struct stat *f_stat;
};

typedef struct f_list_s f_list;
// f_data plus information for the managment of the array
struct f_list_s {
    struct f_data_s **f_data;
    int len;
    int size_i;
};

// Small lemma for the quicksort algorithm.
static inline void swap(struct f_data_s **restrict a, struct f_data_s **restrict b) {
    struct f_data_s *temp = *b;
    *b = *a;
    *a = temp;
}
// Used to determine sorting order, this should only called internally.
int key_order(struct key_s *k1, struct key_s *k2);
// Creates a key for each given file name, this should only be called internally.
int make_key(char *f_name, struct key_s *key);

// Always use to initialize a new f_list.
f_list* f_list_init();
// Always use to initialize a new f_list if you care about the seed (for quicksort obvi)
f_list* f_list_init_seed(unsigned int seed);
// DOES NOT COPY VALUES OF f_name OR f_stat, memory integrity is managed by the user!
int f_list_add_elem(f_list* fl, char *f_name, struct stat *f_stat);
// Called by f_list_add_elem, the f_list_size array determines each increment in the 
//   size of the array each time it's resized.
int _f_list_resize(f_list* fl);
// User function for an in-place sort of an f_list.
void f_list_sort(f_list* fl);
// Quicksort implementation, do not call.
void _f_list_sort_r(f_list* fl, int p, int r);
// Partition function for quicksort, do not call.
int _f_list_partition(f_list* fl, int p, int r);
// Obselete function for doing simple data output
// Removing soon!
int f_list_data_out(f_list* fl, FILE *f);
// Call if all the values you passed in were dynamically allocated
void f_list_delete_ddata(f_list* fl);
// Call if all the values you passed in were statically allocated
// Note: Two values are passed in by the user for each element. If some are 
//   statically allocated and some are dynamically allocated, you're on your own.
void f_list_delete_sdata(f_list* fl);

#endif /* __QUICK_H */