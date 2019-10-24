/**
 * Library for creating and evaluating expressions.
 * 
 * An expression is made up of primaries connected by operators. A primary
 *   is the smallest unit of an expression, and takes a minimum of two values:
 *     -The stat information of a file
 *     -An argument given at the time of the expression's creation
 * Other primaries also need extra information about the state of the program
 *   or system. This information doesn't change during execution.
 * Operators are just logical connectives between primaries, but for now the
 *   only available operator is an implicit && between all primaries. Because
 *   of this, it is suficient that our expressions be represented as a linked
 *   list of primaries. If all are true, the expression is true, and is false
 *   otherwise.
 */

#include "expression.h"

/**
 * Creates an expression from a list of string arguments. Because each primary
 *   must have both the name of the primary and an argument, it creates
 *   each primary by moving two arguments at a time, parsing them and adding
 *   them to the expression list.
 * The user is expected to have initialized and expression_t object that
 *   expression points to.
 * Returns: EXPR_ERR_NONE on success, and any other expr_err value if some
 *   part of the parsing fails and deletes any allocated part of the expression.
 */
expr_err expression_create(expression_t *expression, int expr_argc, \
        char **expr_argv) {
    primary_node *node = NULL;
    expr_err ret = EXPR_ERR_NONE;

    assert(expression != NULL);
    if (get_primary_globals(&(expression->global_args)) < 0) {
        return EXPR_ERR_GLOBALS;
    }
    expression->head = NULL;
    
    int i = 0;
    while (i+1 < expr_argc) {
        ret = expression_create_primary(&node, expr_argv[i], expr_argv[i+1]);
        if (ret != EXPR_ERR_NONE) {
            goto cleanup;
        }
        expression_add_primary(expression, node);
        i += 2;
    }

    if (i+1 == expr_argc) {
        ret = EXPR_ERR_NO_ARG;
        goto cleanup;
    }
    return ret;

    cleanup:
    expression_delete(expression);
    return ret;
}

/**
 * Parses primary_s and primary_arg_s and puts the appropriate values in the
 *   primary_node referenced by node.
 * Returns: EXPR_ERR_NONE on success, and otherwise returns an expr_err
 *   indicating what part of the parsing/allocating process the function failed.
 */
expr_err expression_create_primary(primary_node **node, char *primary_s, \
        char *primary_arg_s) {
    expr_err ret = EXPR_ERR_NONE;
    
    errno = 0;
    *node = malloc(sizeof(primary_node));
    if (*node == NULL) {
        ret = EXPR_ERR_MALLOC;
        goto cleanup;
    }

    if (primary_parse(&((*node)->primary), primary_s) < 0) {
        ret = EXPR_ERR_PRIMARY;
        goto cleanup;
    }

    if (primary_arg_parse((*node)->primary, &((*node)->arg), primary_arg_s)<0) {
        ret = EXPR_ERR_ARG;
        goto cleanup;
    }

    (*node)->next = NULL;
    return ret;

    cleanup:
    if (*node != NULL) {
        free(*node);
        *node = NULL;
    }
    return ret;
}

/**
 * Adds the given primary node to expression. Appends it to the end of the
 *   linked list.
 */
void expression_add_primary(expression_t *expression, primary_node *node) {
    primary_node *curr = expression->head;
    if (curr == NULL) {
        expression->head = node;
        return;
    }

    while (curr->next != NULL) {
        curr = curr->next;
    }
    curr->next = node;
}

/**
 * Evaluates the expression against f_stat.
 * Returns: true if expression evaluates to true for all primaries, false
 *   otherwise
 */
bool expression_evaluate(expression_t *expression, struct stat *f_stat) {
    primary_node *curr = expression->head;
    bool ret = true;

    while (ret && curr != NULL) {
        if (!primary_evaluate(curr->primary, &(curr->arg), \
                &(expression->global_args), f_stat)) {
            ret = false;
        }
        curr = curr->next;
    }
    return ret;
}

/**
 * Deletes the entire expression.
 */
void expression_delete(expression_t *expression) {
    primary_node *next, *curr = expression->head;
    while (curr != NULL) {
        next = curr->next;
        free(curr);
        curr = next;
    }
}