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

/* Determines lexigraphic order of keys k1 and k2
 * Return is less than 0 if k1 is before k2, 0 if k1 == k2, and is greater than
 *   0 if k1 > k2 (determined by characterwise comparisons of the two strings)
 */
int key_order(struct key_s *k1, struct key_s *k2) {
    int ret = strcoll(k1->cmp_char, k2->cmp_char);
    if (ret == 0)
        ret = strcoll(k1->cmp_case, k2->cmp_case);
    return ret;
}

/* TODO: Later
 */
struct key_s* make_key(const char *s) {
    int i, len = strlen(s) + 1;
    struct key_s* s_key;

    errno = 0;
    s_key = malloc(sizeof(struct key_s));
    if (s_key == NULL) {
        return NULL;
    }
    s_key->cmp_char = malloc(sizeof(char) * len);
    if (s_key->cmp_char == NULL) {
        free(s_key);
        return NULL;
    }
    s_key->cmp_case = malloc(sizeof(char) * len);
    if (s_key->cmp_case == NULL) {
        free(s_key->cmp_char);
        free(s_key);
        return NULL;
    }
    
    for (i = 0; i < len; i++) {
        s_key->cmp_char[i] = tolower(s[i]);
        s_key->cmp_case[i] = s[i];
    }
    return s_key;
}

/* Creates and returns an empty tree
 * Returns NULL if allocation of the pointer to tree fails
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

/* Creates and returns new node
 * Returns NULL if allocation of the new node fails
 */
node* _bst_create_node(char *s, struct key_s *s_key, node *parent, const char color) {
    errno = 0;
    node *n = malloc(sizeof(node));
    if (n == NULL) return NULL;
    n->parent = parent;
    n->left = NULL;
    n->right = NULL;
    n->color = color;
    n->data = s;
    n->key = s_key;
    return n;
}

/* Adds a new node with data value s into tree t
 * Function behavior is undefined if the tree t does not conform to red/black properties
 *   i.e. it was not passed in the correct order through functions that preserve the invariants or 
 *   was somehow malformed.
 * Returns: 0 on success, 1 if node already exists, and -1 if some memory allocation failed
 */
int bst_add_node(tree *t, char *s) {
    int ret;
    node *p, *q = NULL;
    struct key_s *s_key = make_key(s);
    if (s_key == NULL)
        return -1;

    assert(t != NULL);
    p = *t;
    while (p != NULL) {
        q = p;
        ret = key_order(s_key, p->key);
        if (ret == 0) {
            free(s_key);
            return 1;
        }
        if (ret < 0)
            p = p->left;
        else /* ret > 0 */
            p = p->right;
    }
    if (q == NULL) {
        *t = _bst_create_node(s, s_key, NULL, BLACK);
        if (*t == NULL) {
            free(s_key->cmp_char);
            free(s_key->cmp_case);
            free(s_key);
            return -1;
        }
        return 0;
    }
    if (ret < 0) {
        q->left = _bst_create_node(s, s_key, q, RED);
        if (q->left == NULL) {
            free(s_key->cmp_char);
            free(s_key->cmp_case);
            free(s_key);
            return -1;
        }
        _bst_fix_order(t, q->left);
    }
    else /* ret > 0 */ {
        q->right = _bst_create_node(s, s_key, q, RED);
        if (q->right == NULL) {
            free(s_key->cmp_char);
            free(s_key->cmp_case);
            free(s_key);
            return -1;
        }
        _bst_fix_order(t, q->right);
    }
    return 0;
}

/* Finds any violations of the red/black property in tree t immediately after 
 *   adding a node.
 * For the purposes of the algorithm, NULL is a node with the color black.
 * Behavior is undefined if the tree given to bst_add_node did not fulfill red/black property.
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
        free(n->key->cmp_char);
        free(n->key->cmp_case);
        free(n->key);
        free(n);
    }
}

/* Deletes tree t with statically allocated data
 */
void bst_delete_tree_sdata(tree *t) {
    _bst_delete_tree_sdata_r(*t);
    free(t);
    t = NULL;
}

/* Recursive implementation for bst_delete_tree_sdata
 */
void _bst_delete_tree_sdata_r(node *n) {
    if (n != NULL) {
        _bst_delete_tree_sdata_r(n->left);
        _bst_delete_tree_sdata_r(n->right);
        free(n->key->cmp_char);
        free(n->key->cmp_case);
        free(n->key);
        free(n);
    }
}