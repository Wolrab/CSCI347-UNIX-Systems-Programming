#include "list.h"

/**
 * Implementation of an increasing-order linked list for holding file names 
 *   and stat information.
 */

/**
 * Initializes a linked list with increasing order. If l is null, a new list
 *   is created
 * Returns: The initialized list on success, NULL if malloc fails.
 */
list* list_init(list *l) {
    list *l_init;

    l_init = l;
    if (l_init == NULL) {
        errno = 0;
        l_init = malloc(sizeof(list));
        if (l_init == NULL) {
            return NULL;
        }
    }

    *l_init = NULL;
    return l_init;
}

/**
 * Creates and adds a node, maintaining increasing order between all nodes 
 *   after the addition.
 * Returns: LIST_ERR_NONE on success, LIST_ERR_MALLOC if malloc fails and
 *   LIST_ERR_DUP_ENTRY if the created node already exists in the list.
 */
list_err list_add_ordered(list *l, char *f_name, struct stat *f_stat) {
    node *curr, *n;
    list_err ret = LIST_ERR_NONE; 
    int ord = 0;

    errno = 0;
    n = malloc(sizeof(node));
    if (n == NULL) {
        return LIST_ERR_MALLOC;
    }

    ret = node_fill_data(&(n->data), f_name, f_stat);
    switch (ret) {
    case LIST_ERR_NONE:
        break;
    case LIST_ERR_MALLOC:
        free(n);
        return ret;
    }

    if (*l == NULL) {
        *l = n;
        n->next = NULL;
        return ret;
    }

    curr = *l;
    ord = node_order(n, curr);
    if (ord < 0) {
        n->next = curr;
        *l = n;
        return ret;
    }

    ord = node_order(n, curr->next);
    while (ord > 0) {
        curr = curr->next;
        ord = node_order(n, curr->next);
    }
    if (ord == 0) {
        free(n->data.f_name);
        free(n->data.f_name_lower);
        free(n);
        ret = LIST_ERR_DUP_ENTRY;
    }
    else {
        n->next = curr->next;
        curr->next = n;
        ret = LIST_ERR_NONE;
    }
    return ret;
}

/**
 * Deletes all nodes of a list and points the list to NULL
 */
void list_delete(list *l) {
    node *curr, *next;

    curr = *l;
    while (curr != NULL) {
        next = curr->next;
        node_delete(curr);
        curr = next;
    }
    *l = NULL;
}

/**
 * Fills the given data struct with data and handles copying of values where
 *   that is needed.
 * Returns: LIST_ERR_NONE on success, LIST_ERR_MALLOC if malloc fails.
 */
list_err node_fill_data(struct data_s *data, char *f_name, struct stat *f_stat) {
    errno = 0;
    data->f_name = malloc(strlen(f_name) + 1);
    if (data->f_name == NULL && errno) {
        return LIST_ERR_MALLOC;
    }
    memcpy(data->f_name, f_name, strlen(f_name) + 1);

    data->f_name_lower = lower_string_cpy(f_name);
    if (data->f_name_lower && errno) {
        free(data->f_name);
        return LIST_ERR_MALLOC;
    }

    data->f_stat = f_stat;
    return LIST_ERR_NONE;
}

/**
 * Deletes a given node.
 */
void node_delete(node *n) {
    free(n->data.f_name);
    free(n->data.f_name_lower);
    free(n->data.f_stat);
    free(n);
}

/**
 * Compares order between two nodes. NULL is defined to have greater order 
 *   than any other node so the increasing-order list invariant is preserved.
 * Returns: > 0 if n1 > n2, < 0 if n1 < n2, and 0 if n1 == n2.
 */
int node_order(node *n1, node *n2) {
    int ret = 0;
    if (n1 == NULL && n2 == NULL) {
        return 0;
    }
    if (n1 == NULL) {
        return 1;
    }
    if (n2 == NULL) {
        return -1;
    }
    ret = strcoll(n1->data.f_name_lower, n2->data.f_name_lower);
    if (ret == 0) {
        ret = strcoll(n1->data.f_name, n2->data.f_name);
    }
    return ret;
}

/**
 * Creates a copy of s with all alphabetical characters turned to their
 *   lowercase form.
 * Returns: New string on success, NULL if malloc fails.
 */
char* lower_string_cpy(char *s) {
    char *cpy;

    errno = 0;
    cpy = malloc(strlen(s) + 1);
    if (cpy == NULL && errno) {
        return NULL;
    }

    for (int i = 0; i < strlen(s) + 1; i++) {
        cpy[i] = tolower(s[i]);
    }

    return cpy;
}
