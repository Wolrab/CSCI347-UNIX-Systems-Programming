#ifndef __LIST_H
#define __LIST_H
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>

typedef struct node_s node;
typedef struct list_s list;

struct data_s {
    char *f_name;
    char *f_name_lower;
    struct stat *f_stat;
};

struct node_s {
    struct data_s data;
    node *next;
};

struct list_s {
    int size;
    node *head;
};

// Initializes the values of l. l must already be allocated.
void list_init(list *l);

// Creates a new node with data specified by f_name and f_stat. The user must
//   ensure f_stat is a valid reference for the entire life of the list.
node* list_create_node(char *f_name, struct stat *f_stat);

// Sets the given data in n.
int node_set_data(node *n, char *f_name, struct stat *f_stat);

// Inserts n into l while preserving increasing order of l.
// If adding the first element of an empty list, that list must be initialized
//   with list_init.
void list_insert_ordered(list *l, node *n);

// Deletes l and points its head to NULL. The user must not be holding any
//   references to data internal to this list after it is deleted.
void list_delete(list *l);

// Deletes n and all its entries.
void node_delete(node *n);

// Determines order between two nodes.
int node_order(node *n1, node *n2);

// Puts a lowercase copy of src into the string pointed at by dest.
int lower_string_cpy(char** dest, char *src);

#endif /* __LIST_H */