#ifndef __DIR_ARR_H
#define __DIR_ARR_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <errno.h>

struct key_s {
    char *cmp_char; // The string set to all lowercase, all non-character comparisons relative to locale are held
    char *cmp_case; // The string itself, for the case that all characters match and casewise comparison is required
};

struct data_s {
    char *data;
    struct key_s key;
};

static const unsigned int arr_size[] = {0x0000000F, 0x000000FF, 0x000003FF, 0x00000FFF, \
                                        0x00003FFF, 0x0000FFFF, 0x0003FFFF, 0x000FFFFF, \
                                        0x003FFFFF, 0x00FFFFFF, 0x03FFFFFF, 0x0FFFFFFF, \
                                        0x3FFFFFFF, 0xFFFFFFFF};
#define N_ARR_SIZE 14

typedef struct dir_ds_s qs;
struct dir_ds_s {
    struct data_s **arr;
    int arr_len;
    int arr_size_i;
};

static inline void swap(struct data_s **a, struct data_s **b);
int key_order(struct key_s *k1, struct key_s *k2);
int make_key(char *s, struct key_s *key);

qs* qs_init_container();
qs* qs_init_container_seed(unsigned int seed);
int qs_add_elem(qs *container, char *s);
int _qs_resize(qs *container);
void qs_sort(qs *container);
void _qs_sort_r(qs *container, int q, int r);
int _qs_partition(qs *container, int q, int r);
int qs_data_out(qs *container, FILE *f);
void qs_delete_ddata(qs *container);
void qs_delete_sdata(qs *container);

#endif /* __QUICK_H */