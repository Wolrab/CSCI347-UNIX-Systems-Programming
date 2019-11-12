/**
 * Interface for creating and evaluating expressions.
 * 
 * An expression is made up of primaries connected by operators. A primary
 *   is the smallest unit of an expression, and takes a minimum of two values:
 *     -Information from a file (in this case from an FTSENT struct)
 *     -One or more arguments given at the time of the expression's creation
 * Some primaries also need extra information about the state of the program.
 *   This information doesn't change during execution.
 * Operators are just logical connectives between primaries, but the only
 *   operator we shall consider is an implicit && between all primaries. Because
 *   of this, it is sufficient that our expressions be represented as a linked
 *   list of primaries. If all are true, the expression is true, and is false
 *   otherwise.
 */
#include "expression.h"

/**
 * Creates an expression from a list of string arguments. Because each primary
 *   has an unknown number of arguments, expression_create_primary is passed
 *   a pointer to the current position in expr_argv and moves it to the next
 *   unparsed position in expr_argv. This is then the position of the next
 *   primary. expr_argv itself is untouched.
 * If expr_argv contains no arguments, the expression is still valid and will
 *   always return true if evaluated.
 * Returns EXPR_ERR_NONE on success, and any other expr_err value if some
 *   part of the parsing fails.
 */
expr_err expression_create(expression_t *expression, char **expr_argv) {
    primary_node *node = NULL;
    char *primary_str = NULL, **primary_arg_i = NULL;
    expr_err ret = EXPR_ERR_NONE;

    assert(expression != NULL);
    if (get_prog_state(&(expression->state_args)) < 0) {
        ret = EXPR_ERR_STATE;
    }
    else {
        expression->head = NULL;
        primary_str = expr_argv[0];
        primary_arg_i = &(expr_argv[1]);
        while (primary_str != NULL && ret == EXPR_ERR_NONE) {
            ret = expression_create_primary(&node, primary_str, \
                &primary_arg_i);
            if (ret != EXPR_ERR_NONE) {
                expression_delete(expression);
            }
            else {
                expression_add_primary(expression, node);

                primary_str = primary_arg_i[0];
                if (primary_arg_i[0] != NULL) {
                    primary_arg_i = &(primary_arg_i[1]);
                }
            }
        }
    }
    return ret;
}

/**
 * Creates and fills a primary node by consuming arguments of primary_arg_i.
 * On success, primary_arg_i points to the next arg after the last consumed
 *   arg. This is accomplished by the actual parsing function this function
 *   calls.
 * Returns EXPR_ERR_NONE on success and any other expr_err on failure
 *   indicating what part of the parsing/allocating process failed.
 */
expr_err expression_create_primary(primary_node **node, char *primary_str, \
        char ***primary_arg_i) {
    expr_err ret = EXPR_ERR_NONE;

    errno = 0;
    *node = malloc(sizeof(primary_node));
    if (*node == NULL) {
        ret = EXPR_ERR_MALLOC;
    }
    else if (primary_parse(&((*node)->primary), primary_str) < 0) {
        ret = EXPR_ERR_PRIMARY;
        free(*node);
        *node = NULL;
    }
    else if ((*primary_arg_i)[0] == NULL) {
        ret = EXPR_ERR_NO_ARG;
        free(*node);
        *node = NULL;
    }
    else if (primary_arg_parse((*node)->primary, &((*node)->arg), \
            primary_arg_i) < 0) {
        ret = EXPR_ERR_ARG;
        free(*node);
        *node = NULL;
    }
    else {
        (*node)->next = NULL;
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
    }
    else {
        while (curr->next != NULL) {
            curr = curr->next;
        }
        curr->next = node;
    }
}

/**
 * Evaluates the expression against entry.
 * Returns true if all primaries in expression evaluate to true, false
 *   otherwise.
 */
bool expression_evaluate(expression_t *expression, FTSENT *entry) {
    primary_node *curr = expression->head;
    bool ret = true;

    while (ret && curr != NULL) {
        if (!primary_evaluate(curr->primary, &(curr->arg), \
                &(expression->state_args), entry)) {
            ret = false;
        }
        else {
            curr = curr->next;
        }
    }
    return ret;
}

/**
 * Deletes the entire expression. If any memory was allocated for the arg, it
 *   is deleted with primary_delete_arg.
 */
void expression_delete(expression_t *expression) {
    primary_node *next, *curr = expression->head;
    while (curr != NULL) {
        next = curr->next;
        primary_delete_arg(curr->primary, &(curr->arg));
        free(curr);
        curr = next;
    }
}