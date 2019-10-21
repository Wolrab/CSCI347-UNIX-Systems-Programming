#ifndef __PATH_LIST_H
#define __PATH_LIST_H
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <errno.h>

// Error defines to make returns meaningful
typedef enum list_err list_err;
enum list_err {
    LIST_ERR_NONE = 0,
    LIST_ERR_MALLOC = 1,
    LIST_ERR_DUP_ENTRY = 2
};

struct data_s {
    char *path;
    char *path_lower;
};

typedef struct node_s node;
typedef node* list;

struct node_s {
    struct data_s data;
    node *next;
};

// Initializes a list. If l is NULL, mallocs a new list. Otherwise 
//   appropriately initializes l.
list* list_init(list *l);

// Creates and adds a new node to l with the given data. Handles any necessary
//   copying.
list_err list_add_ordered(list *l, char *path);

// Deletes l and points it to NULL.
void list_delete(list *l);

// Helper function for list_add_ordered.
// Fills the data struct.
list_err node_fill_data(struct data_s *data, char *path);

// Deletes n and all its entries.
void node_delete(node *n);

// Determines order between two nodes.
int node_order(node *n1, node *n2);

// Makes a copy of s with all the alphabetic characters changed to their 
//   lowercase form.
char* lower_string_cpy(char *s);

#endif /* __PATH_LIST_H */