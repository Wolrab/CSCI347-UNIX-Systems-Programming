/**
 * Implementation of an increasing-order linked list for holding path names. 
 *   The increasing-order property is ensured by creating nodes through 
 *   list_create_node and adding them with list_insert_ordered.
 */
#include "list.h"

/**
 * Creates a node with the given data. path is copied into two strings, one
 *   being an all-lowercase variant for sorting.
 * Returns The new node on success, NULL otherwise.
 */
node* list_create_node(char *path) {
    node *n;
    list_err ret = LIST_ERR_NONE; 

    errno = 0;
    n = malloc(sizeof(node));
    if (n != NULL) {
        ret = node_set_data(n, path);
        if (ret == LIST_ERR_MALLOC) {
            free(n);
            n = NULL;
        }
    }
    return n;
}

/**
 * Fills n's data field, handling allocation and copying of path into
 *   data.path and data.path_lower.
 * Returns LIST_ERR_NONE on success, LIST_ERR_MALLOC if malloc fails.
 */
list_err node_set_data(node *n, char *path) {
    list_err ret = LIST_ERR_NONE;

    errno = 0;
    n->data.path = malloc(strlen(path) + 1);
    if (n->data.path == NULL && errno) {
        ret = LIST_ERR_MALLOC;
    }
    else {
        strncpy(n->data.path, path, strlen(path) + 1);

        ret = lower_string_cpy(&(n->data.path_lower), path);
        if (ret == LIST_ERR_MALLOC) {
            free(n->data.path);
        }
        else {
            n->next = NULL;
        }
    }
    return ret;
}

/**
 * Adds n to the list that maintains the increasing order invariant of the list.
 * If l is an empty list, it must be initialized and point to NULL.
 * Returns LIST_ERR_NONE on success and LIST_ERR_DUP_ENTRY if n already 
 *   exists in the list, though in that case the duplicate entry still will be
 *   stored.
 */
list_err list_insert_ordered(list *l, node *n) {
    node *curr = NULL;
    list_err ret = LIST_ERR_NONE; 
    int ord = 0;

    assert(l != NULL);
    if (*l == NULL) {
        *l = n;
        ret = LIST_ERR_NONE;
    }
    else if (node_order(n, *l) < 0) {
        n->next = *l;
        *l = n;
    }
    else {
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
    }
    return ret;
}

/**
 * Deletes all nodes of l and points l to NULL.
 */
void list_delete(list *l) {
    node *curr = NULL, *next = NULL;

    assert(l != NULL);
    curr = *l;
    while (curr != NULL) {
        next = curr->next;
        node_delete(curr);
        curr = next;
    }
    *l = NULL;
}

/**
 * Deletes n and all its entries.
 */
void node_delete(node *n) {
    free(n->data.path);
    free(n->data.path_lower);
    free(n);
}

/**
 * Compares order between two nodes. NULL is defined to have greater order 
 *   than any other node so the increasing-order list invariant is preserved
 *   in all cases.
 * Returns >0 if n1 > n2, <0 if n1 < n2, and 0 if n1 == n2.
 */
int node_order(node *n1, node *n2) {
    int ret = 0;
    if (n1 == NULL || n2 == NULL) {
        if (n1 == NULL) {
            ret++;
        }
        if (n2 == NULL) {
            ret--;
        }
    }
    else {
        ret = strcoll(n1->data.path_lower, n2->data.path_lower);
        if (ret == 0) {
            ret = strcoll(n1->data.path, n2->data.path);
        }
    }
    return ret;
}

/**
 * Puts a lowercase copy of src into the string pointed at by dest.
 * Returns LIST_ERR_NONE on success, LIST_ERR_MALLOC if malloc fails.
 */
list_err lower_string_cpy(char** dest, char *src) {
    list_err ret = LIST_ERR_NONE;

    errno = 0;
    *dest = malloc(strlen(src) + 1);
    if (*dest == NULL && errno) {
        ret = LIST_ERR_MALLOC;
    }
    else {
        for (int i = 0; i < strlen(src) + 1; i++) {
            (*dest)[i] = tolower(src[i]);
        }
    }
    return ret;
}
