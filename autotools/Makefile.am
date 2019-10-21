bin_PROGRAMS=ls find
ls_SOURCES=ls/ls.c ls/ls.h ls/list.c ls/list.h
find_SOURCES=find/find.c find/find.h find/expr.c find/expr.h find/list.c \
             find/list.h

test_scripts=tests/ls_order  \
             tests/ls_exists \
             tests/ls_multi_path \
             tests/ls_option_all \
             tests/ls_order_case

EXTRA_DIST=${test_scripts}
TESTS=${test_scripts}
