lib_LTLIBRARIES = libefforts.la

libefforts_la_CPPFLAGS = -I. -std=gnu++0x -Wall -I/usr/include/ $(TIPA_CFLAGS)
libefforts_la_SOURCES = common.cpp model.cpp constraint_parser.cpp \
	ppl_adapt.cpp expression.cpp assignment_parser.cpp automaton.cpp \
	expression_parser.cpp constraint.cpp automaton_parser.cpp model_parser.cpp \
	sstate.cpp unique_index.cpp combined_edge.cpp edge.cpp assignment.cpp \
	widened_sstate.cpp statistics.cpp \
	edge_factory.cpp signature.cpp \
	p_model.cpp \
	box_widened_sstate.cpp \
	widened_merge_sstate.cpp widened_sstate_ex.cpp
    ##oct_sstate.cpp box_widened_sstate.cpp dbm_sstate.cpp \
    ##time_abstract_state.cpp dual_sstate.cpp \
    ##param_sstate.cpp ce_model.cpp \
    ##bug_model.cpp efficient_p_model.cpp

libefforts_la_LIBADD = -lppl -lgmp $(TIPA_LIBS)


bin_PROGRAMS = efforts
efforts_CPPFLAGS = -I. -std=gnu++0x -Wall $(TIPA_CFLAGS)
efforts_LDFLAGS = -lefforts -lppl -lgmp $(TIPA_LIBS)
efforts_SOURCES = main.cpp
