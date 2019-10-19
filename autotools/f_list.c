#include "f_list.h"

/** 
 * Stores and sorts information on an arbitrary number of files.
 * Information includes the files name, a key to enable proper sorting, and 
 *   a pointer to the files stat information if needed.
 */


/* Determines the order of k1 relative to k2
 * Returns: <0 if k1<k2, 0 if k1==k2, and >0 if k1>k2
 */
int key_order(struct key_s *k1, struct key_s *k2) {
    int ret = strcoll(k1->cmp_char, k2->cmp_char);
    if (ret == 0)
        ret = strcoll(k1->cmp_case, k2->cmp_case);
    return ret;
}

/* Creates a key for use in the key_order function
 * Returns: 0 on success, 1 on error
 */
int make_key(char *f_name, struct key_s *key, f_list_err *err) {
    int i, len = strlen(f_name) + 1;

    key->cmp_case = f_name;

    errno = 0;
    key->cmp_char = malloc(sizeof(void*) * len);
    if (key->cmp_char == NULL && errno) {
        *err = FL_ERR_MALLOC;
        return 1;
    }

    for (i = 0; i < len; i++) {
        key->cmp_char[i] = tolower(f_name[i]);
    }
    return 0;
}

/* Initializes an empty f_list, and seeds srand with the current time
 * Returns NULL if malloc fails
 */
f_list* f_list_init(f_list *fl) {
    if (fl == NULL) {
        errno = 0;
        f_list* fl = malloc(sizeof(f_list));
        if (fl == NULL && errno) return NULL;
    }

    errno = 0;
    fl->f_data = malloc(sizeof(void*) * _f_list_size[0]);
    if (fl->f_data == NULL && errno) {
        free(fl->f_data);
        free(fl);
        return NULL;
    }
    fl->len = 0;
    fl->size_i = 0;
    fl->err = FL_ERR_NONE;

    srand((unsigned int)time(NULL));
    return fl;
}

/* Initializes an empty f_list, and seeds srand with the given seed value
 * Returns NULL a malloc fails
 */
f_list* f_list_init_seed(f_list *fl, unsigned int seed) {
    if (fl == NULL) {
        errno = 0;
        fl = malloc(sizeof(f_list));
        if (fl == NULL && errno) return NULL;
    }

    errno = 0;
    fl->f_data = malloc(sizeof(void*) * _f_list_size[0]);
    if (fl->f_data == NULL && errno) {
        free(fl->f_data);
        free(fl);
        return NULL;
    }
    fl->len = 0;
    fl->size_i = 0;
    fl->err = FL_ERR_NONE;

    srand(seed);
    return fl;
}

/* Adds a new element to the f_list
 * Returns: 0 on success, 1 on error
 */
int f_list_add_elem(f_list *fl, char *f_name, struct stat *f_stat, char *link_path) {
    int i, ret;

    assert(f_name != NULL);

    if (_f_list_size[fl->size_i] == fl->len) {
        if (_f_list_resize(fl)) return 1;
    }
    i = fl->len;

    errno = 0;
    fl->f_data[i] = malloc(sizeof(struct f_data_s));
    if (fl->f_data[i] == NULL && errno) {
        fl->err = FL_ERR_MALLOC;
        return 1;
    }
    
    ret = make_key(f_name, &(fl->f_data[i]->key), &(fl->err));
    if (ret) return 1;
    memcpy(fl->f_data[i]->f_name, f_name, strlen(f_name)+1);
    fl->f_data[i]->f_stat = f_stat;
    fl->f_data[i]->link_path = link_path;

    fl->len++;

    return 0;
}

/* Resizes the f_list to the next available array size
 * Returns: 0 on success, 1 on error
 */
int _f_list_resize(f_list* fl) {
    int i;
    struct f_data_s **f_data_cpy;

    fl->size_i++;
    if (fl->size_i == _f_list_size_num) {
        fl->err = FL_ERR_ARRAY_OVERFLOW;
        return 1;
    }

    errno = 0;
    f_data_cpy = malloc(sizeof(void*) * _f_list_size[fl->size_i]);
    if (f_data_cpy == NULL && errno) {
        fl->err = FL_ERR_MALLOC;
        return 1;
    }

    for (i = 0; i < fl->len; i++) {
        f_data_cpy[i] = fl->f_data[i];
    }
    fl->f_data = f_data_cpy;
    
    return 0;
}

/* Sorts the f_list with order determined by the key_order function
 */
void f_list_sort(f_list* fl) {
    _f_list_sort_r(fl, 0, fl->len - 1);
}

/* Quicksort function
 */
void _f_list_sort_r(f_list* fl, int s, int r) {
    int q;
    if (s < r) {
        q = _f_list_partition(fl, s, r);
        _f_list_sort_r(fl, s, q-1);
        _f_list_sort_r(fl, q+1, r);
    }
}

/* Partition function for quicksort
 * Returns: Index of sorted pivot element
 */
int _f_list_partition(f_list* fl, int s, int r) {
    int i, j, pivot = (rand() % (r-s+1)) + s;
    struct f_data_s *temp;
    if (pivot != r) SWAP(fl->f_data[pivot], fl->f_data[r], temp)
    pivot = r;
    i = s - 1;
    for (j = s; j < pivot; j++) {
        if (key_order(&(fl->f_data[j]->key), &(fl->f_data[pivot]->key)) < 0) {
            i++;
            if (i < j) SWAP(fl->f_data[i], fl->f_data[j], temp)
        }
    }
    i++;
    if (i < r) SWAP(fl->f_data[i], fl->f_data[pivot], temp)
    return i;
}

/* Deletes an f_list */
void f_list_delete_data(f_list* fl) {
    int i;

    if (fl != NULL) {
        for (i = 0; i < fl->len; i++) {
            free(fl->f_data[i]->key.cmp_char);
            free(fl->f_data[i]->f_stat);
            free(fl->f_data[i]);
        }
    }
}