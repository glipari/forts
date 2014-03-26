#ifndef _SYNTAX_TREES_HPP_
#define _SYNTAX_TREES_HPP_

#include <constraint.hpp>
#include <expression_parser.hpp>
#include <tipa/tinyparser.hpp>
//using namespace std;
using namespace tipa;

/**
   This is a function to build an atomic constraint from a simple
   string. 
 */
constraint_node build_a_constraint_tree(std::string expr_input);

std::shared_ptr<const expr_tree_node> build_an_expr_tree(const std::string &expr_input);

class builder : public expr_builder {
    std::vector< std::shared_ptr<atomic_constraint_node> > at_nodes;
    constraint_node constraint;
public:
    builder () : expr_builder() {}// constraint = nullptr; }
    
    template<class T>
    void store_comp(parser_context &pc) {
	auto x = pc.collect_tokens();
	auto at_node = std::make_shared<T>(); 
        at_nodes.push_back(at_node);
    }
    
    void store_true(parser_context &pc);

    constraint_node get_tree(); 
};

typedef builder constraint_builder;

rule prepare_constraint_rule(constraint_builder &cb);

#endif
