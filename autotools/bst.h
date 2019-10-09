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
    char *val;
    char color;
};

int string_order(char*, char*);

tree* bst_init();
node* _bst_create_node(char*, node*, char);
int bst_add_node(tree*, char*);
void _bst_fix_order(tree*, node*);
void _bst_left_rotate(tree*, node*);
void _bst_right_rotate(tree*, node*);

void bst_inorder_out(tree*, FILE *);
void _bst_inorder_out_r(node*, FILE*);

void bst_delete_tree_ddata(tree*);
void _bst_delete_tree_ddata_r(node*);
void bst_delete_tree_sdata(tree*);
void _bst_delete_tree_sdata_r(node*);

#endif /*__BST_H*/