#include "syntax_trees.hpp"

shared_ptr<atomic_constraint_node> build_an_at_tree(string expr_input)
{
  rule atomic_constraint, expr, primary, term, op_plus, op_minus, op_mult, op_div,
       r_int, r_var, at_l, at_leq, at_eq, at_geq, at_g;
  atomic_constraint = at_leq | at_geq | at_eq | at_l | at_g;

  at_l    = expr > rule("<") > expr;
  at_leq  = expr > rule("<=") > expr;
  at_eq   = expr > rule("==") > expr;
  at_geq  = expr > rule(">=") > expr;
  at_g    = expr > rule(">") > expr;

  //at_l    = expr >> rule("<") >> expr;
  //at_leq  = expr >> rule("<=") >> expr;
  //at_eq   = expr >> rule("==") >> expr;
  //at_geq  = expr >> rule(">=") >> expr;
  //at_g    = expr >> rule(">") >> expr;

  expr = term >> *(op_plus | op_minus);
  op_plus = rule('+') > term;
  op_minus = rule('-') > term;

  term = primary >> *(op_mult | op_div);
  op_mult = rule('*') > primary;
  op_div = rule('/') > primary;

  primary = r_int | r_var | 
    rule('(') >> expr >> rule(')');

  r_int = rule(tk_int);
  r_var = rule(tk_ident);

  builder b;
  using namespace std::placeholders;

  r_var    [std::bind(&builder::make_var,                   &b, _1)];
  r_int    [std::bind(&builder::make_leaf,                  &b, _1)];
  op_plus  [std::bind(&builder::make_op<plus_node>,         &b, _1)];
  op_minus [std::bind(&builder::make_op<minus_node>,        &b, _1)];
  op_mult  [std::bind(&builder::make_op<mult_node>,         &b, _1)];
  op_div   [std::bind(&builder::make_op<div_node>,          &b, _1)];


  at_l      [std::bind(&builder::make_at<l_node>,           &b, _1)];
  at_leq    [std::bind(&builder::make_at<leq_node>,         &b, _1)];
  at_eq     [std::bind(&builder::make_at<eq_node>,          &b, _1)];
  at_geq    [std::bind(&builder::make_at<geq_node>,         &b, _1)];
  at_g      [std::bind(&builder::make_at<g_node>,           &b, _1)];
  

  stringstream str(expr_input);

  parser_context pc;
  pc.set_stream(str);

  bool f = false;
  try {
    f = atomic_constraint.parse(pc);
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
