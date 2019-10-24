#include "expression.h"

/**
 * TODO: Rationale
 */

/**
 * Creates an expression given a sequence of primaries and their arguments.
 * Returns: EXPR_ERR_NONE if full expression is successfully parsed. Otherwise
 *   returns any of the other EXPR_ERR's and permutates expr_args if a parsing 
 *   error occured. Invalid primaries are put at expr_args[0] and invalid 
 *   arguments at expr_args[1].
 */
expr_err expression_create(expression_t *expression, char **expr_args, \
        int expr_args_size) {
    primary_node *node;
    expr_err ret = EXPR_ERR_NONE;

    if (*expression != NULL) {
        *expression = NULL;
    }

    int i = 0;
    while (i < expr_args_size) {
        ret = expression_parse_primary_node(expr_args, expr_args_size, \
            i, &node);
        if (ret != EXPR_ERR_NONE) {
            permutate_args(ret, expr_args, i);
            goto cleanup_list;
        }

        expression_append(expression, node);
        i += 2;
    }
    return ret;

    cleanup_list:
    expression_delete(expression);
    return ret;
}

/**
 * Helper function for permuting expr_args given different errors
 */
void permutate_args(expr_err err, char **expr_args, int i) {
    if (err == EXPR_ERR_INVALID_PRIMARY) {
        permutate_args_invalid_primary(expr_args, i);
    }
    else if (err == EXPR_ERR_INVALID_ARG) {
        permutate_args_invalid_primary_arg(expr_args, i, i+1);
    }
    else if (err == EXPR_ERR_NO_ARG) {
        permutate_args_invalid_primary(expr_args, i);
    }
}

/**
 * Parses one primary and its arg at a given point in expr_args and puts the 
 *   resulting primary_node in node. This is the messy workhorse of 
 *   primary_node creation, though most of its work is managing errors.
 * Returns: EXPR_ERR_NONE on success, otherwise returns any of the other 
 *   EXPR_ERR's.
 */
expr_err expression_parse_primary_node(char **expr_args, int expr_args_size, \
        int expr_arg_i, primary_node **node) {
    primary_t primary;
    *node = NULL;
    expr_err ret = EXPR_ERR_NONE;

    ret = get_primary(expr_args[expr_arg_i], &primary);
    if (ret == EXPR_ERR_INVALID_PRIMARY) {
        goto cleanup;
    }

    errno = 0;
    *node = malloc(sizeof(primary_node));
    if (*node == NULL) {
        ret = EXPR_ERR_MALLOC;
        goto cleanup;
    }

    if (expr_arg_i == expr_args_size-1) {
        ret = EXPR_ERR_NO_ARG;
        goto cleanup;
    }

    ret = primary_node_fill(primary, expr_args[expr_arg_i+1], *node);
    if (ret != EXPR_ERR_NONE) {
        goto cleanup;
    }
    return ret;

    cleanup:
    if (*node != NULL) {
        free(node);
        *node = NULL;
    }
    return ret;
}

/**
 * Evaluates a full expression for its truth value given a file's stat struct.
 * All primaries are assumed to by &'ed together.
 * Returns: true if all primaries return true, false otherwise.
 */
bool expression_evaluate(expression_t *expression, struct stat *f_stat) {
    primary_node *curr;
    bool ret = true;

    curr = *expression;
    while (curr != NULL && ret == true) {
        switch(curr->type) {
        case ARG_LONG:
            ret = curr->primary_arg_type.long_t.eval(f_stat, \
                curr->primary_arg_type.long_t.arg);
            break;
        case ARG_STAT:
            ret = curr->primary_arg_type.stat_t.eval(f_stat, \
                curr->primary_arg_type.stat_t.arg);
            break;
        case ARG_FILE_TYPE:
            ret = curr->primary_arg_type.file_type_t.eval(f_stat, \
                curr->primary_arg_type.file_type_t.arg);
            break;
        }
        curr = curr->next;
    }
    return ret;
}

/**
 * Appends a primary_node to expression
 */
void expression_append(expression_t *expression, primary_node *node) {
    primary_node *curr;

    curr = *expression;
    if (curr == NULL) {
        *expression = node;
        return;
    }

    while (curr->next != NULL) {
        curr = curr->next;
    }
    curr->next = node;
}

/**
 * Deletes an expression
 */
void expression_delete(expression_t *expression) {
    primary_node *curr, *next; 
    
    curr = *expression;
    while (curr != NULL) {
        next = curr->next;
        if (curr->type == ARG_STAT) {
            free(curr->primary_arg_type.stat_t.arg);
        }
        free(curr);
        curr = next;
    }

    *expression = NULL;
}

/**
 * Finds and sets the eval functoin for node.
 * Returns: EXPR_ERR_NONE on success and EXPR_ERR_INVALID_PRIMARY if
 *   primary is not found.
 */
expr_err set_primary_eval_func(primary_t primary, primary_node *node) {
    switch(primary) {
    case CNEWER:
        node->primary_arg_type.stat_t.eval = &eval_cnewer;
        break;
    case CMIN:
        node->primary_arg_type.long_t.eval = &eval_cmin;
        break;
    case CTIME:
        node->primary_arg_type.long_t.eval = &eval_ctime;
        break;
    case MMIN:
        node->primary_arg_type.long_t.eval = &eval_mmin;
        break;
    case MTIME:
        node->primary_arg_type.long_t.eval = &eval_mtime;
        break;
    case TYPE:
        node->primary_arg_type.file_type_t.eval = &eval_type;
        break;
    default:
        return EXPR_ERR_INVALID_PRIMARY;
    }
    return EXPR_ERR_NONE;
}

/**
 * Finds the correct primary by searching primary_str_arr for the string 
 *   matching expr_element.
 * Returns: EXPR_ERR_NONE on success, EXPR_ERR_INVALID_PRIMARY if no 
 *   corresponding primary is found.
 */
expr_err get_primary(char *expr_element, primary_t *primary) {
    int cmp = 1, primary_id;

    primary_id = 0;
    if (strlen(primary_str_arr[primary_id]) == strlen(expr_element)) {
        cmp = strncmp(primary_str_arr[primary_id], expr_element, \
                strlen(expr_element));
    }
    while (cmp != 0 && primary_id < PRIMARIES_TOTAL) {
        if (strlen(primary_str_arr[primary_id]) == strlen(expr_element)) {
            cmp = strncmp(primary_str_arr[primary_id], expr_element, \
                    strlen(expr_element));
        }
        if (cmp != 0) {
            primary_id++;
        }
    } 

    if (primary_id == PRIMARIES_TOTAL) {
        return EXPR_ERR_INVALID_PRIMARY;
    }

    *primary = (primary_t)primary_id;
    return EXPR_ERR_NONE;
}

/**
 * Parses expr_element and sets node's arg value accordingly.
 * Returns: EXPR_ERR_NONE on success, EXPR_ERR_INVALID_ARG if the type-specific
 *   arg function fails and EXPR_ERR_MALLOC for a malloc failure.
 */
expr_err get_primary_arg(char *expr_element, primary_node *node) {
    expr_err ret = EXPR_ERR_NONE;

    switch(node->type) {
    case ARG_LONG:
        ret = get_long_arg(&(node->primary_arg_type.long_t.arg), expr_element);
        break;
    case ARG_STAT:
        ret = get_stat_arg(&(node->primary_arg_type.stat_t.arg), expr_element);
        break;
    case ARG_FILE_TYPE:
        ret = get_file_type_arg(&(node->primary_arg_type.file_type_t.arg), \
                expr_element);
        break;
    }
    return ret;
}

/**
 * Parser for the ARG_LONG arg_type.
 * Returns: EXPR_ERR_NONE if long_arg is a string consisting of base-10
 *   digits whose numeric representation fits in a long. Returns 
 *   EXPR_ERR_INVALID_ARG otherwise.
 */
expr_err get_long_arg(long *dest, char *long_arg) {
    long result;
    if (!is_number(long_arg)) {
        return EXPR_ERR_INVALID_ARG;
    }
    errno = 0;
    result = strtol(long_arg, NULL, 10);
    if (errno == ERANGE) {
        return EXPR_ERR_INVALID_ARG;
    }
    *dest = result;
    return EXPR_ERR_NONE;
}

/**
 * Parser for the ARG_STAT arg_type.
 * Returns: EXPR_ERR_NONE if path_arg is a stat-able path. Returns 
 *   EXPR_ERR_INVALID_ARG if that is not the case and EXPR_ERR_MALLOC if 
 *   malloc fails.
 */
expr_err get_stat_arg(struct stat **dest, char *path_arg) {
    struct stat *f_stat;
    
    errno = 0;
    f_stat = malloc(sizeof(struct stat));
    if (f_stat == NULL) {
        return EXPR_ERR_MALLOC;
    }
    if (stat(path_arg, f_stat) < 0) {
        free(f_stat);
        return EXPR_ERR_INVALID_ARG;
    }
    *dest = f_stat;
    return EXPR_ERR_NONE;
}

/**
 * Parser for the ARG_FILE_TYPE arg_type
 * Returns: EXPR_ERR_NONE if file_type_arg contains a single, null-terminated, 
 *   file_type-compliant character. Returns EXPR_ERR_INVALID_ARG otherwise.
 */
expr_err get_file_type_arg(char *dest, char *file_type_arg) {
    if (strlen(file_type_arg) != 1) {
        return EXPR_ERR_INVALID_ARG;
    }
    int i = 0;
    while (i < strlen(valid_file_types) && \
           file_type_arg[0] != valid_file_types[i]) {
        i++;
    }
    if (i == strlen(valid_file_types)) {
        return EXPR_ERR_INVALID_ARG;
    }
    *dest = file_type_arg[0];
    return EXPR_ERR_NONE;
}

/**
 * Swaps primary_i and element 0 to allow for better error visibility in the
 *   calling function.
 */
void permutate_args_invalid_primary(char **expr_args, int primary_i) {
    swap(expr_args, 0, primary_i);
}

/**
 * Swaps primary_i and element 0 and primary_arg_i and element 1 to allow for
 *   better error visibility in the calling function.
 */
void permutate_args_invalid_primary_arg(char **expr_args, int primary_i, int primary_arg_i) {
    swap(expr_args, 0, primary_i);
    swap(expr_args, 1, primary_arg_i);
}

/**
 * Simple swapping function.
 */
void swap(char **arr, int i, int j) {
    char *temp = arr[i];
    arr[i] = arr[j];
    arr[j] = temp;
}

/**
 * Small utility to have stricter detection of malformed longs.
 * Returns: True if all characters of arg are base 10 digits, false otherwise.
 */
bool is_number(char *arg) {
    int len = strlen(arg);
    int i = 0;
    while (i < len && (arg[i] >= '0' && arg[i] <= '9')) {
        i++;
    }
    if (i < len) {
        return false;
    }
    return true;
}