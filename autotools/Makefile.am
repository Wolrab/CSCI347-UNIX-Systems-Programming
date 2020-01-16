bin_PROGRAMS=ls find
ls_SOURCES=ls_src/ls.c ls_src/list.c ls_src/list.h ls_src/long_out.c \
		   ls_src/long_out.h ls_src/print_utils.c ls_src/print_utils.h
find_SOURCES=find_src/find.c find_src/expression.c find_src/expression.h \
			 find_src/expression_prim_parse.c find_src/expression_prim_parse.h \
             find_src/expression_prim_eval.c find_src/expression_prim_eval.h \
			 find_src/expression_prim_defs.h find_src/list.c find_src/list.h

test_scripts=tests/find_cnewer   \
             tests/find_exec     \
             tests/find_exists   \
             tests/find_type     \
             tests/ls_exists     \
             tests/ls_multi_path \
             tests/ls_option_a   \
             tests/ls_option_d   \
             tests/ls_option_i   \
             tests/ls_option_l   \
             tests/ls_order      \
             tests/ls_order_case

EXTRA_DIST=${test_scripts}
TESTS=${test_scripts}
