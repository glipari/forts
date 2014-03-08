#include "expression.hpp"

#include <tipa/tinyparser.hpp>
#include "syntax_trees.hpp"

using namespace std;
using namespace tipa;

class expr_builder {
    stack< shared_ptr<expr_tree_node> > st;
    // vector< shared_ptr<atomic_constraint_node> > at_nodes;
    // shared_ptr<constraint_node> constraint;
public:
    expr_builder () {}

    void make_leaf(parser_context &pc){
	auto x = pc.collect_tokens();
	if (x.size() < 1) throw string("Error in collecting integer.");
	int v = atoi(x[x.size()-1].second.c_str());
	auto node = make_shared<expr_leaf_node>(v);
	st.push(node);
    }
    
    template<class T>
    void make_op(parser_context &pc) {
	auto r = st.top(); st.pop();
	auto l = st.top(); st.pop();
	auto n = make_shared<T>();
	n->set_left(l);
	n->set_right(r);
	st.push(n);
    }
    
    void make_var(parser_context &pc) {
	auto x = pc.collect_tokens();
	if (x.size() < 1) throw string("Error in collecting variable."); 
	string v = x[x.size()-1].second;
	auto node = make_shared<expr_var_node>(v);
	st.push(node);
    }
    
    int get_size() {return st.size();}
    
    // template<class T>
    // void store_comp(parser_context &pc) {
    // 	auto x = pc.collect_tokens();
    // 	auto at_node = make_shared<T>(); 
    //     at_nodes.push_back(at_node);
    // }
    
    // shared_ptr<constraint_node> get_tree() {
    //   if ( constraint != nullptr) {
    //     return constraint;
    //   }
    //   constraint = make_shared<constraint_node>();
    //   for ( auto it = at_nodes.rbegin(); it != at_nodes.rend(); it ++) {
    //     //auto it = at_nodes.back();
    // 	auto r = st.top(); st.pop();
    // 	auto l = st.top(); st.pop();
    //     (*it)->set_left(l);
    //     (*it)->set_right(r);
    //   }

    //   for ( auto it = at_nodes.begin(); it != at_nodes.end(); it ++)
    //     constraint->append_atomic_constraint(*it);
    //   return constraint;
    // }

    shared_ptr<expr_tree_node> get_tree() {
	return st.top();
    }
};

shared_ptr<expr_tree_node> build_expression(const string &expr_input)
{
    rule expr, primary, term, 
	op_plus, op_minus, op_mult, op_div,
	r_int, r_var; 
	// at_l, at_leq, at_eq, at_geq, at_g;
    
    // constraint = atomic_constraint >> *( rule('&') >> atomic_constraint);
    // rule comparison = at_leq | at_geq | at_eq | at_l | at_g;
    // atomic_constraint = expr > comparison > expr;
    // at_l    = rule("<", true);
    // at_leq  = rule("<=", true);
    // at_eq   = rule("==", true);
    // at_geq  = rule(">=", true);
    // at_g    = rule(">", true);

    expr = term >> *(op_plus | op_minus);
    op_plus = rule('+') > term;
    op_minus = rule('-') > term;

    term = primary >> *(op_mult | op_div);
    op_mult = rule('*') > primary;
    op_div = op_mult;

    primary = r_int | r_var | 
	rule('(') >> expr >> rule(')');

    r_int = rule(tk_int);
    r_var = rule(tk_ident);

    expr_builder b;
    using namespace std::placeholders;

    r_var    [std::bind(&expr_builder::make_var,                   &b, _1)];
    r_int    [std::bind(&expr_builder::make_leaf,                  &b, _1)];
    op_plus  [std::bind(&expr_builder::make_op<plus_node>,         &b, _1)];
    op_minus [std::bind(&expr_builder::make_op<minus_node>,        &b, _1)];
    op_mult  [std::bind(&expr_builder::make_op<mult_node>,         &b, _1)];

    // at_l      [std::bind(&builder::store_comp<l_node>,        &b, _1)];
    // at_leq    [std::bind(&builder::store_comp<leq_node>,      &b, _1)];
    // at_eq     [std::bind(&builder::store_comp<eq_node>,       &b, _1)];
    // at_geq    [std::bind(&builder::store_comp<geq_node>,      &b, _1)];
    // at_g      [std::bind(&builder::store_comp<g_node>,        &b, _1)];
  
    stringstream str(expr_input);

    parser_context pc;
    pc.set_stream(str);

    bool f = false;
    try {
	// //f = atomic_constraint.parse(pc);
	f = expr.parse(pc);
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
