lib_LTLIBRARIES = libefforts.la

libefforts_la_CPPFLAGS = -I. -std=c++11 -Wall $(TIPA_CFLAGS)
libefforts_la_SOURCES = model.cpp
#LIBS = -ltinyparser
libefforts_la_LIBADD = $(TIPA_LIBS)


bin_PROGRAMS = efforts
efforts_CPPFLAGS = -I. -std=c++11 -Wall $(TIPA_CFLAGS)
efforts_LDFLAGS = -lefforts $(TIPA_LIBS)
efforts_SOURCES = main.cpp
