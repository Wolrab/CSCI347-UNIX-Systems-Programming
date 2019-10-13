#include "dir_arr.h"

/* A data structure to hold directory information and sort it
 */ 

int key_order(struct key_s *k1, struct key_s *k2) {
    int ret = strcoll(k1->cmp_char, k2->cmp_char);
    if (ret == 0)
        ret = strcoll(k1->cmp_case, k2->cmp_case);
    return ret;
}

int make_key(char *s, struct key_s *key) {
    int i, len = strlen(s) + 1;

    key->cmp_case = s;

    errno = 0;
    key->cmp_char = malloc(sizeof(void*) * len);
    if (key->cmp_char == NULL) return -1;

    for (i = 0; i < len; i++) {
        key->cmp_char[i] = tolower(s[i]);
    }
    return 0;
}

static inline void swap(struct data_s **a, struct data_s **b) {
    struct data_s *temp = *b;
    *b = *a;
    *a = temp;
}

qs* qs_init_container() {
    qs *container = malloc(sizeof(qs));
    container->arr_size_i = -1;
    container->arr_len = 0;
    if (_qs_resize(container) < 0) {
        free(container);
        return NULL;
    }

    srand((unsigned int)time(NULL));
    return container;
}

qs* qs_init_container_seed(unsigned int seed) {
    srand(seed);
    return qs_init_container();
}

/* 
 * Returns: 0 if normal, 1 if maximum size reached, -1 if malloc error
 */
int qs_add_elem(qs *container, char *s) {
    int i, ret, err;

    container->arr_len++;
    if (arr_size[container->arr_size_i] + 1 == container->arr_len) {
        ret = _qs_resize(container);
        if (ret != 0) return ret;
    }
    i = container->arr_len - 1;
    
    errno = 0;
    container->arr[i] = malloc(sizeof(struct data_s));
    if (container->arr[i] == NULL) return -1;

    if (make_key(s, &(container->arr[i]->key)) < 0) {
        err = errno;
        free(container->arr[i]);
        container->arr[i] = NULL;
        errno = err;
        return -1;
    }
    container->arr[i]->data = s;
    
    return 0;
}

/* Returns: 0 if normal, 1 if maximum size reached, -1 if malloc error
 */
int _qs_resize(qs *container) {
    int i;
    struct data_s **arr_cpy;

    container->arr_size_i++;
    if (container->arr_size_i == N_ARR_SIZE) return 1;
    
    errno = 0;
    arr_cpy = malloc(sizeof(void*) * arr_size[container->arr_size_i]);
    if (arr_cpy == NULL) return -1;

    for (i = 0; i < container->arr_len; i++) {
        arr_cpy[i] = container->arr[i];
    }
    container->arr = arr_cpy;
    return 0;
}

void qs_sort(qs *container) {
    _qs_sort_r(container, 0, container->arr_len - 1);
}

void _qs_sort_r(qs *container, int p, int r) {
    int q;
    if (p < r) {
        q = _qs_partition(container, p, r);
        _qs_sort_r(container, p, q-1);
        _qs_sort_r(container, q+1, r);
    }
}

int _qs_partition(qs *container, int p, int r) {
    int i, j, pivot = (rand() % (r-p+1)) + p;
    if (pivot != r) swap(&(container->arr[pivot]), &(container->arr[r]));
    i = p - 1;
    for (j = p; j < r; j++) {
        if (key_order(&(container->arr[j]->key), &(container->arr[r]->key)) < 0) {
            i++;
            if (i < j) swap(&(container->arr[i]), &(container->arr[j]));
        }
    }
    i++;
    if (i < r) swap(&(container->arr[i]), &(container->arr[r]));
    return i;
}

int qs_data_out(qs *container, FILE *f) {
    int i;
    for (i = 0; i < container->arr_len; i++) {
        if (fprintf(f, "%s\n", container->arr[i]->data) < 0) {
            return -1;
        }
    }
    return 0;
}

void qs_delete_ddata(qs *container) {
    int i;
    for (i = 0; i < container->arr_len; i++) {
        free(container->arr[i]->key.cmp_char);
        free(container->arr[i]->data);
        free(container->arr[i]);
    }
    free(container);
}

void qs_delete_sdata(qs *container) {
    int i;
    for (i = 0; i < container->arr_len; i++) {
        free(container->arr[i]->key.cmp_char);
        free(container->arr[i]);
    }
    free(container);
}