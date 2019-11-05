/**
 * Library for creating and evaluating expressions.
 * 
 * An expression is made up of primaries connected by operators. A primary
 *   is the smallest unit of an expression, and takes a minimum of two values:
 *     -The stat information of a file
 *     -One or more arguments given at the time of the expression's creation
 * Other primaries also need extra information about the state of the program
 *   or system. This information doesn't change during execution.
 * Operators are just logical connectives between primaries, but for now the
 *   only available operator is an implicit && between all primaries. Because
 *   of this, it is sufficient that our expressions be represented as a linked
 *   list of primaries. If all are true, the expression is true, and is false
 *   otherwise.
 *
 * Note to future self: When expression's functionality is expanded to include
 *   more operators and parenthesized sub-expressions, I believe that
 *   expression_t can be changed very easily to represent some kind of binary
 *   parse-tree with primary_node's as leaves and operators connecting them.
 */
#include "expression.h"

/**
 * Creates an expression from a list of string arguments. Because each primary
 *   must have both the name of the primary and one or more arguments, 
 *   expression_create_primary moves primary_arg_i to the next index after the
 *   last parsed arg, and this is the position of the start of the next primary.
 *   On an error, primary_arg_i is not moved and cleanup of any already
 *   processed elements is done.
 * Returns: EXPR_ERR_NONE on success, and any other expr_err value if some
 *   part of the parsing fails.
 */
expr_err expression_create(expression_t *expression, int expr_argc, \
        char **expr_argv) {
    primary_node *node = NULL;
    char *primary_str = NULL, **primary_arg_i = NULL;
    expr_err ret = EXPR_ERR_NONE;

    assert(expression != NULL);
    if (get_primary_globals(&(expression->global_args)) < 0) {
        return EXPR_ERR_GLOBALS;
    }

    expression->head = NULL;
    if (expr_argv[0] == NULL) {
        return EXPR_ERR_NONE;
    }
    
    primary_str = expr_argv[0];
    primary_arg_i = &(expr_argv[1]);
    while (primary_str != NULL) {
        ret = expression_create_primary(&node, primary_str, &primary_arg_i);
        if (ret != EXPR_ERR_NONE) {
            goto cleanup;
        }
        expression_add_primary(expression, node);

        primary_str = primary_arg_i[0];
        if (primary_arg_i[0] != NULL) {
            primary_arg_i = &(primary_arg_i[1]);
        }
    }
    return ret;

    cleanup:
    expression_delete(expression);
    return ret;
}

/**
 * TODO:
 * Returns: EXPR_ERR_NONE on success, and otherwise returns an expr_err
 *   indicating what part of the parsing/allocating process failed.
 */
expr_err expression_create_primary(primary_node **node, char *primary_str, \
        char ***primary_arg_i) {
    expr_err ret = EXPR_ERR_NONE;

    errno = 0;
    *node = malloc(sizeof(primary_node));
    if (*node == NULL) {
        ret = EXPR_ERR_MALLOC;
        goto cleanup;
    }
    
    if (primary_arg_i == NULL) {
        ret = EXPR_ERR_NO_ARG;
        goto cleanup;
    }

    if (primary_parse(&((*node)->primary), primary_str) < 0) {
        ret = EXPR_ERR_PRIMARY;
        goto cleanup;
    }

    if (primary_arg_parse((*node)->primary, &((*node)->arg), primary_arg_i)<0) {
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
 * Adds the given primary_node to expression by appending it to the final node
 *   in expression, preserving the order the expression will be evaluated in.
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
 * Evaluates the expression against entry.
 * Returns: true if expression evaluates to true for all primaries, false
 *   otherwise
 */
bool expression_evaluate(expression_t *expression, FTSENT *entry) {
    primary_node *curr = expression->head;
    bool ret = true;

    while (ret && curr != NULL) {
        if (!primary_evaluate(curr->primary, &(curr->arg), \
                &(expression->global_args), entry)) {
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