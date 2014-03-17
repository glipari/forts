#ifndef _SYNTAX_TREES_HPP_
#define _SYNTAX_TREES_HPP_

#include <constraint.hpp>
#include <expression_parser.hpp>
#include <tipa/tinyparser.hpp>
using namespace std;
using namespace tipa;

/**
   This is a function to build an atomic constraint from a simple
   string. 
 */
std::shared_ptr<constraint_node> build_a_constraint_tree(std::string expr_input);

std::shared_ptr<expr_tree_node> build_an_expr_tree(const std::string &expr_input);

class builder : public expr_builder {
    vector< shared_ptr<atomic_constraint_node> > at_nodes;
    shared_ptr<constraint_node> constraint;
public:
    builder () : expr_builder() { constraint = nullptr; }
    
    template<class T>
    void store_comp(parser_context &pc) {
	auto x = pc.collect_tokens();
	auto at_node = make_shared<T>(); 
        at_nodes.push_back(at_node);
    }
    
    shared_ptr<constraint_node> get_tree() {
	if ( constraint != nullptr) {
	    return constraint;
	}
	constraint = make_shared<constraint_node>();
	for ( auto it = at_nodes.rbegin(); it != at_nodes.rend(); it ++) {
	    auto r = st.top(); st.pop();
	    auto l = st.top(); st.pop();
	    (*it)->set_left(l);
	    (*it)->set_right(r);
	}

	for ( auto it = at_nodes.begin(); it != at_nodes.end(); it ++)
	    constraint->append_atomic_constraint(*it);
	return constraint;
    }
};

typedef builder constraint_builder;

rule prepare_constraint_rule(constraint_builder &cb);

#endif
