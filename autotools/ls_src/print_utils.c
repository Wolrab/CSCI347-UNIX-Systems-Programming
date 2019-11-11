/**
 * An interface to map various system types to their hard-coded, system-specific
 *   format strings, and then get the maximum number of characters that a value
 *   of that format type could take up. Makes magic numbers much less magic.
 */
#include "print_utils.h"

/**
 * Gets the maximum number of characters that the given format string could take
 *   up when processed. This is only defined for decimal types that this program
 *   processes in a stat struct plus a couple others.
 * The asserts are included to make this job much easier, as these values are
 *   universal, but if one of these does not hold the program will fail and
 *   display which assertion was broken.
 * Returns the number of characters on success, and -1 if the format string is
 *   unknown by this function.
 */
int get_f_max_strlen(char *format) {
    assert(INT_MAX == 2147483647);
    assert(UINT_MAX == 4294967295);
    assert(LONG_MAX == 9223372036854775807L);
    assert(ULONG_MAX == 18446744073709551615UL);
    assert(LLONG_MAX == 9223372036854775807LL);
    assert(ULLONG_MAX == 18446744073709551615ULL);

    static int int_max_strlen = 12;    // "-2147483648"
    static int uint_max_strlen = 11;   // "4294967295"
    static int long_max_strlen = 21;   // "-9223372036854775808"
    static int ulong_max_strlen = 21;  // "18446744073709551615"
    static int llong_max_strlen = 21;  // "-9223372036854775808"
    static int ullong_max_strlen = 21; // "18446744073709551615"

    if (strncmp(format, "%d", 3) == 0) {
        return int_max_strlen;
    }
    else if (strncmp(format, "%u", 3) == 0) {
        return uint_max_strlen;
    }
    else if (strncmp(format, "%ld", 4) == 0) {
        return long_max_strlen;
    }
    else if (strncmp(format, "%lu", 4) == 0) {
        return ulong_max_strlen;
    }
    else if (strncmp(format, "%lld", 5) == 0) {
        return llong_max_strlen;
    }
    else if (strncmp(format, "%llu", 5) == 0) {
        return ullong_max_strlen;
    }
    else {
        return -1;
    }
}