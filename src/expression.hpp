#ifndef _EXPRESSION_HPP_
#define _EXPRESSION_HPP_

#include "syntax_trees.hpp"

class expression {
    std::shared_ptr<expr_tree_node> root; 
public:
    expression (): root(nullptr) {}
};


#endif
