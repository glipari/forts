#ifndef __ASSIGNMENT_PARSER_HPP__
#define __ASSIGNMENT_PARSER_HPP__

#include <tipa/tinyparser.hpp>
#include <common.hpp>
#include <constraint_parser.hpp>
#include <assignment.hpp>

class assignment_builder { 
public:
    // Assignment a;
    std::string var;
    // std::shared_ptr<const expr_tree_node> expr;

    expr_builder b;
    void var_name(tipa::parser_context &pc);
    Assignment get_assignment();
};

Assignment build_assignment(const std::string &str);
tipa::rule prepare_assignment_rule(assignment_builder &ab);

#endif
