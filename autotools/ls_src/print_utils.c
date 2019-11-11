#include "print_utils.h"

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