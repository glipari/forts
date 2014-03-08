#include <tipa/tinyparser.hpp>
#include "syntax_trees.hpp"

using namespace std;
using namespace tipa;


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
	    //auto it = at_nodes.back();
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

shared_ptr<constraint_node> build_a_constraint_tree(string expr_input)
{
    builder b;

    rule constraint, atomic_constraint; //, expr, primary, term, 
	// op_plus, op_minus, op_mult, op_div,
	// r_int, r_var, 
    rule at_l, at_leq, at_eq, at_geq, at_g;
    
    constraint = atomic_constraint >> *( rule('&') >> atomic_constraint);
    rule comparison = at_leq | at_geq | at_eq | at_l | at_g;
    rule expr = prepare_expr_rule(b);

    atomic_constraint = expr > comparison > expr;
    at_l    = rule("<", true);
    at_leq  = rule("<=", true);
    at_eq   = rule("==", true);
    at_geq  = rule(">=", true);
    at_g    = rule(">", true);

    using namespace std::placeholders;

    at_l      [std::bind(&builder::store_comp<l_node>,        &b, _1)];
    at_leq    [std::bind(&builder::store_comp<leq_node>,      &b, _1)];
    at_eq     [std::bind(&builder::store_comp<eq_node>,       &b, _1)];
    at_geq    [std::bind(&builder::store_comp<geq_node>,      &b, _1)];
    at_g      [std::bind(&builder::store_comp<g_node>,        &b, _1)];
  
    stringstream str(expr_input);

    parser_context pc;
    pc.set_stream(str);

    bool f = false;
    try {
	f = constraint.parse(pc);
    } catch (parse_exc &e) {
	cout << "Parse exception!" << endl;
    }

    if (!f) {
	cout << pc.get_formatted_err_msg();
    } else {
	auto tr = b.get_tree();
	return tr;
    }

    return nullptr;
}
