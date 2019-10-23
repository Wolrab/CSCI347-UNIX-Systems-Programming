#include "list.h"

/**
 * Implementation of an increasing-order linked list for holding file names 
 *   and stat information. Overhauled from last time to provide a very 
 *   straightforward sorting solution.
 */

/**
 * Initializes a linked list with increasing order. If l is null, a new list
 *   is created.
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
 * Creates a node with the given data.
 * TODO: RETURN
 */
node* list_create_node(char *f_name, struct stat *f_stat) {
    node *n;
    list_err ret = LIST_ERR_NONE; 

    errno = 0;
    n = malloc(sizeof(node));
    if (n == NULL) {
        return NULL;
    }

    ret = node_set_data(n, f_name, f_stat);
    switch (ret) {
    case LIST_ERR_NONE:
        break;
    case LIST_ERR_MALLOC:
        free(n);
        n = NULL;
    }

    return n;
}

/**
 * Adds n to the list that maintains the increasing order invariant of the list.
 * Returns: LIST_ERR_NONE on success and LIST_ERR_DUP_ENTRY if n already 
 *   exists in the list.
 */
list_err list_insert_ordered(list *l, node *n) {
    node *curr;
    list_err ret = LIST_ERR_NONE; 
    int ord = 0;

    if (*l == NULL) {
        *l = n;
        n->next = NULL;
        return ret;
    }

    ord = node_order(n, *l);
    if (ord < 0) {
        n->next = *l;
        *l = n;
        return ret;
    }

    curr = *l;
    ord = node_order(n, curr->next);
    while (ord > 0) {
        curr = curr->next;
        ord = node_order(n, curr->next);
    }
    if (ord == 0) {
        ret = LIST_ERR_DUP_ENTRY;
    }
    n->next = curr->next;
    curr->next = n;
    return ret;
}

/**
 * Fills n with data, making copies of f_name while leaving f_stat as that data
 *   is already created and managed by the user.
 * Returns: LIST_ERR_NONE on success, LIST_ERR_MALLOC if malloc fails.
 */
list_err node_set_data(node *n, char *f_name, struct stat *f_stat) {
    errno = 0;
    n->data.f_name = malloc(strlen(f_name) + 1);
    if (n->data.f_name == NULL && errno) {
        return LIST_ERR_MALLOC;
    }
    memcpy(n->data.f_name, f_name, strlen(f_name) + 1);

    n->data.f_name_lower = lower_string_cpy(f_name);
    if (n->data.f_name_lower && errno) {
        free(n->data.f_name);
        return LIST_ERR_MALLOC;
    }

    n->data.f_stat = f_stat;
    n->next = NULL;
    return LIST_ERR_NONE;
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
