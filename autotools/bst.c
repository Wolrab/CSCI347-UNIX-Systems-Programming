/* An implementation of a BST with the red/black balancing property. In this case, BST stands
     for Binary Sort Tree, as it is used exclusively for its sorting property and lacks much 
     of the expected (read, required) functionality of a normal BST (things like a search function 
     or the ability to remove a given node). What it DOES do is creates a BST, adds nodes while
     preserving BST and red/black balancing properties, and then does an inorder out of all its 
     elements to a given file when you're done. And then it deletes itself. Nice.

   This implementation was all in a misguided effort to change the necessary string comparisons from 
     an average n/2 to approximately log(n) for n strings, but about halfway through I realized a nice heapsort 
     or quicksort algorithm with logarithmically increasing array sizes during the storage phase 
     would have been a much better alternative. And so the sunk-cost fallacy claims another student's soul.
   
   Reference for the algorithm was sourced from pages 308-322 of the infamous "Introduction to Algorithms"
     C-implementation done by Connor Barlow, esq. */

#include "bst.h"

/* Determines order of strings s1 and s2 (kept as a seperate function for easy modification)
 */
int string_order(char const *s1, char const *s2) {
    return -strcoll(s1, s2);
}

/* Creates an empty tree
 * On error returns NULL
 */
tree* bst_init() {
    errno = 0;
    tree *t = malloc(sizeof(void*));
    if (t == NULL) {
        return NULL;
    }
    *t = NULL;
    return t;
}

/* Creates a new node
 * If malloc fails, bst_add_node catches this since users SHOULD NEVER call this function anyways
 */
node* _bst_create_node(char *s, node *parent, const char color) {
    errno = 0;
    node *n = malloc(sizeof(node));
    if (n == NULL) return NULL;
    n->parent = parent;
    n->left = NULL;
    n->right = NULL;
    n->color = color;
    n->data = s;
    return n;
}

/* Adds a new node with data value s into tree t
 * Returns: 0 on success, 1 if node already exists, and -1 if allocation of new nodes failed
 */
int bst_add_node(tree *t, char *s) {
    int ret;
    node *p, *q = NULL;
    assert(t != NULL);
    p = *t;
    while (p != NULL) {
        q = p;
        ret = string_order(p->data, s);
        if (ret == 0)
            return 1;
        if (ret < 0)
            p = p->left;
        else /* ret > 0 */
            p = p->right;
    }
    if (q == NULL) {
        *t = _bst_create_node(s, NULL, BLACK);
        if (t == NULL)
            return -1;
        return 0;
    }
    if (ret < 0) {
        q->left = _bst_create_node(s, q, RED);
        if (q->left == NULL)
            return -1;
        _bst_fix_order(t, q->left);
    }
    else /* ret > 0 */ {
        q->right = _bst_create_node(s, q, RED);
        if (q->right == NULL)
            return -1;
        _bst_fix_order(t, q->right);
    }
    return 0;
}

/* Finds any violations of the red/black property in tree t immediately after 
 *   adding a node.
 * For the purposes of the algorithm, NULL is a node with the color black.
 */
void _bst_fix_order(tree *t, node *n) {
    node *q;
    while (n->parent != NULL && n->parent->color == RED) {
        if (n->parent == n->parent->parent->left) {
            q = n->parent->parent->right;
            if (q != NULL && q->color == RED) {
                n->parent->color = BLACK;
                q->color = BLACK;
                n->parent->parent->color = RED;
                n = n->parent->parent;
            }
            else {
                if (n == n->parent->right) {
                    _bst_left_rotate(t, n->parent);
                    n = n->left;
                }
                n->parent->color = BLACK;
                n->parent->parent->color = RED;
                _bst_right_rotate(t, n->parent->parent);
            }
        }
        else /* n->parent == n->parent->parent->right */ {
            q = n->parent->parent->left;
            if (q != NULL && q->color == RED) {
                n->parent->color = BLACK;
                q->color = BLACK;
                n->parent->parent->color = RED;
                n = n->parent->parent;
            }
            else {
                if (n == n->parent->left) {
                    _bst_right_rotate(t, n->parent);
                    n = n->right;
                }
                n->parent->color = BLACK;
                n->parent->parent->color = RED;
                _bst_left_rotate(t, n->parent->parent);
            }
        }
    }
    if (n->parent == NULL && n->color == RED)
        n->color = BLACK;
}

/* Performs a left rotation on node n of tree t
 */
void _bst_left_rotate(tree *t, node *n) {
    node* q;
    assert(n->right != NULL);
    q = n->right;
    n->right = q->left;
    if (n->right != NULL)
        n->right->parent = n;
    q->left = n;
    if (n->parent == NULL) {
        *t = q;
        q->parent = NULL;
    }
    else {
        q->parent = n->parent;
        if (n == n->parent->left)
            n->parent->left = q;
        else // n == n->parent->right
            n->parent->right = q;
    }
    n->parent = q;
}

/* Performs a right rotation on node n of tree t
 */
void _bst_right_rotate(tree *t, node *n) {
    node* q;
    assert(n->left != NULL);
    q = n->left;
    n->left = q->right;
    if (n->left != NULL)
        n->left->parent = n;
    q->right = n;
    if (n->parent == NULL) {
        *t = q;
        q->parent = NULL;
    }
    else {
        q->parent = n->parent;
        if (n == n->parent->left)
            n->parent->left = q;
        else // n == n->parent->right
            n->parent->right = q;
    }
    n->parent = q;
}

/* Performs an inorder out of tree t to file f
 */
void bst_inorder_out(tree const *t, FILE *f) {
    _bst_inorder_out_r(*t, f);
}

/* Recursive implementation for bst_inorder_out
 */
void _bst_inorder_out_r(node const *n, FILE *f) {
    if (n != NULL) {
        _bst_inorder_out_r(n->left, f);
        if (fprintf(f, "%s\n", n->data) < 0) {
            perror("fprintf");
            fprintf(stderr, "Output to file failed\n");

        }
        _bst_inorder_out_r(n->right, f);
    }
}

/* Deletes tree t with dynamically allocated data
 */
void bst_delete_tree_ddata(tree *t) {
    _bst_delete_tree_ddata_r(*t);
    free(t);
}

/* Recursive implementation for bst_delete_tree_ddata
 */
void _bst_delete_tree_ddata_r(node *n) {
    if (n != NULL) {
        _bst_delete_tree_ddata_r(n->left);
        _bst_delete_tree_ddata_r(n->right);
        free(n->data);
        free(n);
    }
}

/* Deletes tree t with statically allocated data
 */
void bst_delete_tree_sdata(tree *t) {
    _bst_delete_tree_sdata_r(*t);
    free(t);
}

/* Recursive implementation for bst_delete_tree_sdata
 */
void _bst_delete_tree_sdata_r(node *n) {
    if (n != NULL) {
        _bst_delete_tree_sdata_r(n->left);
        _bst_delete_tree_sdata_r(n->right);
        free(n);
    }
}