bin_PROGRAMS=ls
ls_SOURCES=ls.c f_list.c ls.h f_list.h

test_scripts=tests/ls_order  \
			 tests/ls_exists

TESTS=tests/ls_order  \
      tests/ls_exists \
	  tests/ls_multi_path \
	  tests/ls_option_all \
	  tests/ls_order_case
EXTRA_DIST=${test_scripts}
TESTS=${test_scripts}
