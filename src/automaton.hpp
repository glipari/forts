#ifndef _AUTOMATON_HPP_
#define _AUTOMATON_HPP_

#include <tipa/tinyparser.hpp>
#include "assignment.hpp"
#include "syntax_trees.hpp"

class edge {
public:
    std::shared_ptr<constraint_node> guard;
    std::vector<assignment> assignments;
    std::string dest;
};

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

rule prepare_edge_rule(edge_builder &e_builder);
rule prepare_assignment_rule(edge_builder &e_builder);



class location {
public:
    std::string name; 
    std::shared_ptr<constraint_node> invariant;
    std::vector<assignment> rates;
    std::vector<edge> outgoings;
};

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

rule prepare_location_rule(location_builder &loc_builder);
rule prepare_rate_rule(location_builder &loc_builder);
rule prepare_outgoing_rule(location_builder &loc_builder);
location build_a_location(const std::string &str);

#endif
