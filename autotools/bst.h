#ifndef __BST_H
#define __BST_H
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BLACK 0
#define RED 1

typedef struct node_s node;
typedef node* tree;

struct node_s {
    tree parent;
    tree left;
    tree right;
    char *data;
    char color;
};

int string_order(const char *s1, const char *s2);

// Initializes a pointer to a tree. Necessary for almost all BST functions.
tree* bst_init();
node* _bst_create_node(char *s, node *parent, const char color);
// Function for adding string s and a corresponding node to tree t. The memory of s must be
//   handled by the user of this function!
int bst_add_node(tree *t, char *s);
void _bst_fix_order(tree *t, node *n);
void _bst_left_rotate(tree *t, node *n);
void _bst_right_rotate(tree *t, node *n);

void bst_inorder_out(tree const *t, FILE *f);
void _bst_inorder_out_r(node const *n, FILE *f);

// Delete a BST with dynamically allocated data
void bst_delete_tree_ddata(tree *t);
void _bst_delete_tree_ddata_r(node *n);
void bst_delete_tree_sdata(tree *t);
void _bst_delete_tree_sdata_r(node *n);

#endif /*__BST_H*/