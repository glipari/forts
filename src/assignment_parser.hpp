#ifndef __ASSIGNMENT_HPP__
#define __ASSIGNMENT_HPP__

#include <tipa/tinyparser.hpp>
#include <common.hpp>
#include <constraint_parser.hpp>

class assignment_builder { 
public:
    assignment a;
    expr_builder b;
    void var_name(tipa::parser_context &pc);
    assignment get_assignment();
};

assignment build_assignment(const std::string &str);
tipa::rule prepare_assignment_rule(assignment_builder &ab);

#endif
