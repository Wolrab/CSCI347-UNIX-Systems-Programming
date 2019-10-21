#include "expr.h"

const char *const expr_primaries_str[] = {"cmin", "cnewer", "ctime", "cmin", \
    "mtime", "type"};
const arg_type expr_primaries_arg_type[] = {INT, FILE, INT, INT, INT, CHAR};

const char *const expr_operators_str = {"a"};
/**
 * Returns: EXPR_ERR_NONE on success, and either EXPR_ERR_INVALID_PRIMARY 
 *   or EXPR_ERR_INVALID_ARG if the expression is invalid. It also points 
 *   expression to the primary the error occured on.
 */
expr_err parse_expr(char **expr, int expr_len) {
    
}