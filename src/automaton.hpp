#ifndef _AUTOMATON_HPP_
#define _AUTOMATON_HPP_

#include <tipa/tinyparser.hpp>
#include "assignment.hpp"
#include "syntax_trees.hpp"

class EDGE {
public:
    std::shared_ptr<constraint_node> constraint;
    std::vector<assignment> assignments;
    std::string dest;
};

EDGE build_an_edge(const std::string &str);

class edge_builder {
public:
    EDGE e;
    assignment_builder ass_builder;
    constraint_builder c_builder;
    EDGE get_edge();

    void an_assignment(tipa::parser_context &pc);
    void the_constraint(tipa::parser_context &pc);
    void the_dest(tipa::parser_context &pc);
};

rule prepare_edge_rule(edge_builder &e_builder);
rule prepare_assignment_rule(edge_builder &e_builder);

#endif
