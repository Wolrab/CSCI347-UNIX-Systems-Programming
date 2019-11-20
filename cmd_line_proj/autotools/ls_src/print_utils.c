/**
 * An interface to map various system types to their hard-coded, system-specific
 *   format strings, and then get the maximum number of characters that a value
 *   of that format type could take up. Makes magic numbers much less magic.
 */
#include "print_utils.h"

/**
 * Maximum string lengths for each given max_val. The maximum for a signed
 *   type is determined by its minimum value since the negative sign will be
 *   printed, and we know this minimum given its maximum. The comments above
 *   are included for clarity.
 */
// strlen("-2147483648")+1
const int max_strlen_4bytes_s = 12;
// strlen("4294967295")+1
const int max_strlen_4bytes_u = 11;
// strlen("-9223372036854775808")+1
const int max_strlen_8bytes_s = 21;
// strlen("18446744073709551615")+1
const int max_strlen_8bytes_u = 21;

/**
 * Gets the maximum number of characters that the given format string could take
 *   up when processed. This is only defined for decimal types that this program
 *   processes in a stat struct.
 * An int, as far as I know, must be 4 bytes, so the asserts are used to state
 *   this assumption. long and long long are the trickier ones, so we check if
 *   they are either 4 or 8 bytes and if neither are true we just abort.
 * Returns the number of characters on success, and -1 if the format string is
 *   unknown by this function.
 */
int get_f_max_strlen(char *format) {
    int max_str_len = 0;

    if (strncmp(format, "%d", 3) == 0) {
        assert(INT_MAX == MAX_VAL_4BYTES_S);
        max_str_len = max_strlen_4bytes_s;
    }
    else if (strncmp(format, "%u", 3) == 0) {
        assert(UINT_MAX == MAX_VAL_4BYTES_U);
        max_str_len = max_strlen_4bytes_s;
    }
    else if (strncmp(format, "%ld", 4) == 0) {
        if (LONG_MAX == MAX_VAL_4BYTES_S) {
            max_str_len = max_strlen_4bytes_s;
        }
        else if (LONG_MAX == MAX_VAL_8BYTES_S) {
            max_str_len = max_strlen_8bytes_s;
        }
        else {
            abort();
        }
    }
    else if (strncmp(format, "%lu", 4) == 0) {
        if (ULONG_MAX == MAX_VAL_4BYTES_U) {
            max_str_len = max_strlen_4bytes_u;
        }
        else if (ULONG_MAX == MAX_VAL_8BYTES_S) {
            max_str_len = max_strlen_8bytes_u;
        }
        else {
            abort();
        }
    }
    else if (strncmp(format, "%lld", 5) == 0) {
        if (LLONG_MAX == MAX_VAL_4BYTES_S) {
            max_str_len = max_strlen_4bytes_s;
        }
        else if (LLONG_MAX == MAX_VAL_8BYTES_S) {
            max_str_len = max_strlen_8bytes_s;
        }
        else {
            abort();
        }
    }
    else if (strncmp(format, "%llu", 5) == 0) {
        if (ULLONG_MAX == MAX_VAL_4BYTES_U) {
            max_str_len = max_strlen_4bytes_u;
        }
        else if (ULLONG_MAX == MAX_VAL_8BYTES_U) {
            max_str_len = max_strlen_8bytes_u;
        }
        else {
            abort();
        }
    }
    else {
        max_str_len = -1;
    }
    return max_str_len;
}