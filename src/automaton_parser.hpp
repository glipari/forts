#ifndef __AUTOMATON_PARSER_HPP__
#define __AUTOMATON_PARSER_HPP__

#include <automaton.hpp>
#include <constraint_parser.hpp>
#include <assignment_parser.hpp>

Edge build_an_edge(const std::string &str);

class edge_builder {
public:
    //edge e;
    assignment_builder ass_builder;
    constraint_builder c_builder;

    std::string dest;
    std::string label;
    constraint_node guard;
    std::vector<Assignment> ass;

    void an_assignment(tipa::parser_context &pc);
    void the_guard(tipa::parser_context &pc);
    void the_dest(tipa::parser_context &pc);
    void the_sync_label(tipa::parser_context &pc);

    Edge get_edge();
};

tipa::rule prepare_edge_rule(edge_builder &e_builder);
tipa::rule prepare_assignment_rule(edge_builder &e_builder);

class location_builder {
public:
    // location loc;
    bool bad = false;
    std::string name; 
    constraint_node invariant;
    std::vector<Assignment> rates;
    std::vector<Edge> outgoings;

    assignment_builder rate_builder;
    constraint_builder invariant_builder;
    edge_builder outgoing_builder;

    Location get_location();

    void a_rate(tipa::parser_context &pc);
    void an_outgoing(tipa::parser_context &pc);
    void the_invariant(tipa::parser_context &pc);
    void the_name(tipa::parser_context &pc);
};

tipa::rule prepare_location_rule(location_builder &loc_builder);
tipa::rule prepare_rate_rule(location_builder &loc_builder);
tipa::rule prepare_outgoing_rule(location_builder &loc_builder);
Location build_a_location(const std::string &str);

class automaton_builder {
public:
    //automaton aton;
    std::vector<std::string> labels;
    std::vector<Location>    locations;
    std::string name;

    location_builder l_builder;

    void a_label(tipa::parser_context &pc);
    void the_name(tipa::parser_context &pc);
    void a_location(tipa::parser_context &pc);
    automaton get_automaton();
};

tipa::rule prepare_automaton_rule(automaton_builder &a_builder);
automaton build_an_automaton(const std::string &pc);
#endif
