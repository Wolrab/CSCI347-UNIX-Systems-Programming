/**
 * Implementation of an increasing-order linked list that holds a file names as
 *   well as an optional stat struct. The increasing-order property is ensured
 *   by creating nodes through list_create_node and adding them with
 *   list_insert_ordered.
 */
#include "list.h"

/**
 * Initializes the values of list l. l must already be allocated, and 
 *   initialization mustbe done to ensure size is actually the number of nodes
 *   in l.
 */
void list_init(list *l) {
    l->size = 0;
    l->head = NULL;
}

/**
 * Creates a node with the given data. f_name is copied into two strings, one
 *   being an all-lowercase variant for sorting. The user is expected to ensure
 *   the reference to f_stat is valid for the entire lifetime of the list.
 * Returns The new node on success, NULL if memory allocation failed.
 */
node* list_create_node(char *f_name, struct stat *f_stat) {
    node *n;

    errno = 0;
    n = malloc(sizeof(node));
    if (n != NULL) {
        if (node_set_data(n, f_name, f_stat) < 0) {
            free(n);
            n = NULL;
        }
    }
    return n;
}

/**
 * Fills n's data field, handling allocation and copying of f_name into
 *   data.f_name and data.f_name_lower. f_stat is stored directly.
 * Returns 0 on success, -1 if memory allocation failed.
 */
int node_set_data(node *n, char *f_name, struct stat *f_stat) {
    int ret = 0;

    errno = 0;
    n->data.f_name = malloc(strlen(f_name) + 1);
    if (n->data.f_name == NULL && errno) {
        ret = -1;
    }
    else {
        strncpy(n->data.f_name, f_name, strlen(f_name) + 1);

        ret = lower_string_cpy(&(n->data.f_name_lower), f_name);
        if (ret < 0) {
            free(n->data.f_name);
        }
        else {
            n->data.f_stat = f_stat;
            n->next = NULL;
        }
    }
    return ret;
}

/**
 * Adds n to the list and maintains the increasing order invariant of the list.
 *   The size element of the list is updated here as well.
 */
void list_insert_ordered(list *l, node *n) {
    node *curr = NULL;
    int ord = 0;

    assert(l != NULL);
    if (l->head == NULL) {
        l->head = n;
    }
    else if (node_order(n, l->head) < 0) {
        n->next = l->head;
        l->head = n;
    }
    else {
        curr = l->head;
        ord = node_order(n, curr->next);
        while (ord > 0) {
            curr = curr->next;
            ord = node_order(n, curr->next);
        }
        n->next = curr->next;
        curr->next = n;
    }
    l->size++;
}

/**
 * Deletes all nodes of l and points its head to NULL.
 */
void list_delete(list *l) {
    node *curr = NULL, *next = NULL;

    assert(l != NULL);
    curr = l->head;
    while (curr != NULL) {
        next = curr->next;
        node_delete(curr);
        curr = next;
    }
    l->size = 0;
    l->head = NULL;
}

/**
 * Deletes n and all its entries.
 */
void node_delete(node *n) {
    free(n->data.f_name);
    free(n->data.f_name_lower);
    free(n->data.f_stat);
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
        ret = strcoll(n1->data.f_name_lower, n2->data.f_name_lower);
        if (ret == 0) {
            ret = strcoll(n1->data.f_name, n2->data.f_name);
        }
    }
    return ret;
}

/**
 * Puts a lowercase copy of src into the string pointed at by dest.
 * Returns 0 on success and -1 if malloc fails.
 */
int lower_string_cpy(char** dest, char *src) {
    int ret = 0;

    errno = 0;
    *dest = malloc(strlen(src) + 1);
    if (*dest == NULL && errno) {
        ret = -1;
    }
    else {
        for (int i = 0; i < strlen(src) + 1; i++) {
            (*dest)[i] = tolower(src[i]);
        }
    }
    return ret;
}
