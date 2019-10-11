#ifndef __BST_H
#define __BST_H
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

// Constants for red/black invariant
#define BLACK 0
#define RED 1

typedef struct node_s node;
typedef node* tree;

// Node structure for BST
struct node_s {
    tree parent;
    tree left;
    tree right;
    char *data;
    struct key_s *key;
    char color;
};

// A structure used as the sorting key to make case-insensitive comparison with strcoll work
//   (I checked the locale many times and googling yielded no results for doing case-insensitive comparisons 
//    within the bounds of strcoll itself, so I came up with this solution that preserves the nice locale-
//    dependent features of strcoll while still recognizing the differences between upper and 
//    lowercase letters when necessary (i.e. all letters are the same except some have inverted cases
//    relative to one another))
struct key_s {
    char *cmp_char; // The string set to all lowercase, all non-character comparisons relative to locale are held
    char *cmp_case; // The string itself, for the case that all characters match and casewise comparison is required
};

// Given two keys created with make_key, returns the order of k1 relative to k2
int key_order(struct key_s *k1, struct key_s *k2);

// Given a string s, creates a key compatable with key_order
struct key_s* make_key(const char *s);

// Initializes a pointer to a tree. You must create your tree with this function
tree* bst_init();

// Creates a new node with the given struct values, should only ever be used by bst_add_node
node* _bst_create_node(char *s, struct key_s *s_key, node *parent, const char color);

// Function for adding string s and a corresponding node to tree t. The memory of s must be
//   handled by the caller of this function!
int bst_add_node(tree *t, char *s);

// Maintains the red/black property in tree t. Assumptions about the state of the tree are 
//   baked into the function, should only ever be called by bst_add_node
void _bst_fix_order(tree *t, node *n);

// Makes a left rotation of the node pointed to by n, with the tree reference to the root 
//   included if the node in question is the root. Should only ever be called by _bst_fix_order
void _bst_left_rotate(tree *t, node *n);

// Makes a right rotation of the node pointed to by n, with the tree reference to the root 
//   included if the node in question is the root. Should only ever be called by _bst_fix_order
void _bst_right_rotate(tree *t, node *n);

// Helper function for a recursive inorder out. No change can happen to the tree.
void bst_inorder_out(tree const *t, FILE *f);
// Recursive inorder out. Never call this.
void _bst_inorder_out_r(node const *n, FILE *f);

// Helper function for recursively deleting a tree with dynamically allocated data.
void bst_delete_tree_ddata(tree *t);
// Recursive tree deletion. Never call this.
void _bst_delete_tree_ddata_r(node *n);

// Helper function for recursively deleting a tree with statically allocated data.
void bst_delete_tree_sdata(tree *t);
// Recursive tree deletion. Never call this.
void _bst_delete_tree_sdata_r(node *n);

#endif /*__BST_H*/