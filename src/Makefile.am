lib_LTLIBRARIES = libefforts.la

libefforts_la_CPPFLAGS = -I. -std=gnu++0x -Wall -I/usr/include/ $(TIPA_CFLAGS)
libefforts_la_SOURCES = common.cpp model.cpp syntax_trees.cpp \
	ppl_adapt.cpp expression.cpp assignment.cpp automaton.cpp
#LIBS = -ltinyparser
libefforts_la_LIBADD = -lppl -lgmp $(TIPA_LIBS)


bin_PROGRAMS = efforts
efforts_CPPFLAGS = -I. -std=gnu++0x -Wall $(TIPA_CFLAGS)
efforts_LDFLAGS = -lefforts -lppl -lgmp $(TIPA_LIBS)
efforts_SOURCES = main.cpp
