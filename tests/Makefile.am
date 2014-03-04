TESTS = test
check_PROGRAMS = test

test_CPPFLAGS = -I../src/ -std=gnu++0x -Wall $(TIPA_CFLAGS)
test_LDFLAGS = -L../src/ -lefforts -lppl -lgmp $(TIPA_LIBS)
test_SOURCES = test_main.cpp test_constraint_tree.cpp test_constraint_to_ppl.cpp test_ppl_coefficient.cpp #test_expr_tree.cpp
#LIBS = -ltinyparser
