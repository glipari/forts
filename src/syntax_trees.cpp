#include "syntax_trees.hpp"




shared_ptr<expr_tree_node> build_an_expr_tree(string expr_input)
{
    rule expr, primary, term, 
	op_plus, op_minus, op_mult, //op_div,
	r_int, r_var, 
	at_l, at_leq, at_eq, at_geq, at_g;
    
    expr = term >> *(op_plus | op_minus);
    op_plus = rule('+') > term;
    op_minus = rule('-') > term;

    term = primary >> *op_mult;
    op_mult = rule('*') > primary;
    //op_div = rule('/') > primary;

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

  
    stringstream str(expr_input);

    parser_context pc;
    pc.set_stream(str);

    bool f = false;
    try {
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

rule prepare_constraint_rule(constraint_builder &b)
{
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

    return constraint;
}

shared_ptr<constraint_node> build_a_constraint_tree(string expr_input)
{
    builder b;
    rule constraint = prepare_constraint_rule(b);
  
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


