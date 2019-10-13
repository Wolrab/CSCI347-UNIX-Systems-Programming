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
 * Returns: 0 on success, -1 if malloc fails
 */
int make_key(char *f_name, struct key_s *key) {
    int i, len = strlen(f_name) + 1;

    key->cmp_case = f_name;

    errno = 0;
    key->cmp_char = malloc(sizeof(void*) * len);
    if (key->cmp_char == NULL) return -1;

    for (i = 0; i < len; i++) {
        key->cmp_char[i] = tolower(f_name[i]);
    }
    return 0;
}

/* Initializes an empty f_list, and seeds srand with the current time
 * Returns NULL if malloc fails
 */
f_list* f_list_init() {
    errno = 0;
    f_list* fl = malloc(sizeof(f_list));
    if (fl == NULL) return NULL;

    fl->f_data = malloc(sizeof(void*) * _f_list_size[0]);
    if (fl->f_data == NULL) {
        free(fl->f_data);
        free(fl);
        return NULL;
    }
    fl->len = 0;
    fl->size_i = 0;

    srand((unsigned int)time(NULL));
    return fl;
}

/* Initializes an empty f_list, and seeds srand with the given seed value
 * Returns NULL if malloc fails
 */
f_list* f_list_init_seed(unsigned int seed) {
    errno = 0;
    f_list* fl = malloc(sizeof(f_list));
    if (fl == NULL) return NULL;

    fl->f_data = malloc(sizeof(void*) * _f_list_size[0]);
    if (fl->f_data == NULL) {
        free(fl->f_data);
        free(fl);
        return NULL;
    }
    fl->len = 0;
    fl->size_i = 0;

    srand(seed);
    return fl;
}

/* Adds a new element to the f_list
 * Returns: 0 on success, 1 if maximum array size reached and -1 if malloc fails
 */
int f_list_add_elem(f_list *fl, char *f_name, struct stat *f_stat) {
    int i, ret;

    assert(f_name != NULL);

    if (_f_list_size[fl->size_i] == fl->len) {
        ret = _f_list_resize(fl);
        if (ret != 0) return ret;
    }
    i = fl->len;
    
    errno = 0;
    fl->f_data[i] = malloc(sizeof(struct f_data_s));
    if (fl->f_data[i] == NULL) return -1;

    if (make_key(f_name, &(fl->f_data[i]->key)) < 0) {
        free(fl->f_data[i]);
        fl->f_data[i] = NULL;
        return -1;
    }
    fl->f_data[i]->f_name = f_name;
    fl->f_data[i]->f_stat = f_stat;

    fl->len++;

    return 0;
}

/* Resizes the f_list once it reaches capacity
 * Returns: 0 on success, 1 if maximum f_list size is reached and 
 *   -1 if malloc fails
 */
int _f_list_resize(f_list* fl) {
    int i;
    struct f_data_s **f_data_cpy;

    fl->size_i++;
    if (fl->size_i == _F_LIST_SIZE_LEN) return 1;

    errno = 0;
    f_data_cpy = malloc(sizeof(void*) * _f_list_size[fl->size_i]);
    if (f_data_cpy == NULL) return -1;

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
void _f_list_sort_r(f_list* fl, int p, int r) {
    int q;
    if (p < r) {
        q = _f_list_partition(fl, p, r);
        _f_list_sort_r(fl, p, q-1);
        _f_list_sort_r(fl, q+1, r);
    }
}

/* Partition function for quicksort
 * Returns: Index of sorted pivot element
 */
int _f_list_partition(f_list* fl, int p, int r) {
    int i, j, pivot = (rand() % (r-p+1)) + p;
    if (pivot != r) swap(&(fl->f_data[pivot]), &(fl->f_data[r]));
    i = p - 1;
    for (j = p; j < r; j++) {
        if (key_order(&(fl->f_data[j]->key), &(fl->f_data[r]->key)) < 0) {
            i++;
            if (i < j) swap(&(fl->f_data[i]), &(fl->f_data[j]));
        }
    }
    i++;
    if (i < r) swap(&(fl->f_data[i]), &(fl->f_data[r]));
    return i;
}

/* !!!Obselete!!!
 * Prints the f_name values seperated by newlines to the specified file
 * Returns: 0 on success, -1 if fprintf failed
 */
int f_list_data_out(f_list* fl, FILE *f) {
    int i;
    for (i = 0; i < fl->len; i++) {
        if (fprintf(f, "%s, %s\n", fl->f_data[i]->f_name, (fl->f_data[i]->f_stat == NULL ? "NULL" : "NOT_NULL")) < 0) {
            return -1;
        }
    }
    return 0;
}

/* Deletes an f_list and assumes all data given was dynamically allocated
 */
void f_list_delete_ddata(f_list* fl) {
    int i;

    if (fl != NULL) {
        for (i = 0; i < fl->len; i++) {
            free(fl->f_data[i]->key.cmp_char);
            free(fl->f_data[i]->f_name);
            free(fl->f_data[i]->f_stat);
            free(fl->f_data[i]);
        }
        free(fl);
    }
}

/* Deletes an f_list and assumes all data given was statically allocated
 */
void f_list_delete_sdata(f_list* fl) {
    int i;
    for (i = 0; i < fl->len; i++) {
        free(fl->f_data[i]->key.cmp_char);
        free(fl->f_data[i]);
    }
    free(fl);
}