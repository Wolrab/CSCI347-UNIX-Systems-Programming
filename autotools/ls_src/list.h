#ifndef __LIST_H
#define __LIST_H
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <errno.h>

typedef struct node_s node;
typedef node* list;

struct data_s {
    char *f_name;
    char *f_name_lower;
    struct stat *f_stat;
};

struct node_s {
    struct data_s data;
    node *next;
};

// Error defines
typedef enum list_err list_err;
enum list_err {
    LIST_ERR_NONE = 0,
    LIST_ERR_MALLOC = 1,
    LIST_ERR_DUP_ENTRY = 2
};


// Initializes a list. If l is NULL, mallocs a new list. Otherwise 
//   appropriately initializes l.
list* list_init(list *l);

// Creates a new node with data specified by f_name and f_stat. The user must
//   ensure f_stat is a valid reference for the entire life of the list.
node* list_create_node(char *f_name, struct stat *f_stat);

// Inserts n into l while preserving increasing order of l. Duplicate nodes
//   will be added, but this the caller will be informed of such by 
//   LIST_ERR_DUP_ENTRY being returned.
// If nodes are added to l by any other means than this function, then 
//   list_insert_ordered has undefined behavior.
list_err list_insert_ordered(list *l, node *n);

// Sets the given data in n.
list_err node_set_data(node *n, char *f_name, struct stat *f_stat);

// Deletes l and points it to NULL.
// All members of l, whether they were copied by l or their reference was
//   simply held, will be deleted. The user must not be holding any 
//   references to data internal to this list after it is deleted.
void list_delete(list *l);

// Deletes n and all its entries.
void node_delete(node *n);

// Determines order between two nodes.
int node_order(node *n1, node *n2);

// Makes a copy of s with all the alphabetic characters changed to their 
//   lowercase form.
char* lower_string_cpy(char *s);

#endif /* __LIST_H */