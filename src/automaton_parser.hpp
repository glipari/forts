#ifndef __AUTOMATON_PARSER_HPP__
#define __AUTOMATON_PARSER_HPP__

#include <automaton.hpp>
#include <constraint_parser.hpp>
#include <assignment_parser.hpp>

edge build_an_edge(const std::string &str);

class edge_builder {
public:
    edge e;
    assignment_builder ass_builder;
    constraint_builder c_builder;
    edge get_edge();

    void an_assignment(tipa::parser_context &pc);
    void the_guard(tipa::parser_context &pc);
    void the_dest(tipa::parser_context &pc);
};

tipa::rule prepare_edge_rule(edge_builder &e_builder);
tipa::rule prepare_assignment_rule(edge_builder &e_builder);

class location_builder {
public:
    location loc;
    assignment_builder rate_builder;
    constraint_builder invariant_builder;
    edge_builder outgoing_builder;
    location get_location();

    void a_rate(tipa::parser_context &pc);
    void an_outgoing(tipa::parser_context &pc);
    void the_invariant(tipa::parser_context &pc);
    void the_name(tipa::parser_context &pc);
};

tipa::rule prepare_location_rule(location_builder &loc_builder);
tipa::rule prepare_rate_rule(location_builder &loc_builder);
tipa::rule prepare_outgoing_rule(location_builder &loc_builder);
location build_a_location(const std::string &str);

#endif
