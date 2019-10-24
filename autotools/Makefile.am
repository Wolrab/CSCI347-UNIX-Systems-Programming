bin_PROGRAMS=ls find
ls_SOURCES=ls_src/ls.c ls_src/ls.h ls_src/list.c ls_src/list.h \
           ls_src/long_out.c ls_src/long_out.h
find_SOURCES=find_src/find.c find_src/find.h find_src/expression.c \
             find_src/expression.h find_src/expression_primaries.c \
             find_src/expression_primaries.h find_src/list.c find_src/list.h

test_scripts=tests/find_cnewer \
             tests/find_exists \
             tests/find_type \
             tests/ls_exists \
             tests/ls_option_all \
             tests/ls_order \
             tests/ls_order_case

EXTRA_DIST=${test_scripts}
TESTS=${test_scripts}
