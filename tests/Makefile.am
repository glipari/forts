TESTS = test
check_PROGRAMS = test

test_CPPFLAGS = -I../src/ -std=c++11 -Wall $(TIPA_CFLAGS)
test_LDFLAGS = -L../src/ -lefforts $(TIPA_LIBS)
test_SOURCES = test_main.cpp test_constraint_tree.cpp #test_expr_tree.cpp
#LIBS = -ltinyparser
