#ifndef __ASSIGNMENT_HPP__
#define __ASSIGNMENT_HPP__

#include <common.hpp>
#include <expression.hpp>

/**
   This class represents an assignment.
 */
struct assignment {
    std::string x;
    std::shared_ptr<expr_tree_node> expr;
};

assignment build_assignment(const std::string &str);

#endif
