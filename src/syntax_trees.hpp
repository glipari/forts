#ifndef _SYNTAX_TREES_HPP_
#define _SYNTAX_TREES_HPP_

#include <string>
#include <stack>
#include <sstream>
#include <memory>

#include <tipa/tinyparser.hpp>

using namespace std;
using namespace tipa;

class expr_tree_node {
public : 
  virtual int compute() = 0;
  virtual ~expr_tree_node() {}
};

class expr_op_node : public expr_tree_node {
protected:
  shared_ptr<expr_tree_node> left;
  shared_ptr<expr_tree_node> right;
public:
  void set_left(shared_ptr<expr_tree_node> l){
    left = l;
  }
  void set_right(shared_ptr<expr_tree_node> r){
    right = r;
  }
};

class expr_var_node : public expr_tree_node {
  string name;
  int value;
public:
  expr_var_node(const string &n) : name(n), value(0){}
  virtual int compute() {return value;}
};

class expr_leaf_node : public expr_tree_node {
  int value;
public:
  expr_leaf_node(int v) : value(v){}
  virtual int compute() {return value;}
};

#define EXPR_OP_NODE_CLASS(xxx,sym) \
  class xxx##_node : public expr_op_node {  \
    public:                                 \
      virtual int compute() {               \
        int l = left->compute();            \
        int r = right->compute();           \
        return l sym r;                     \
      }                                     \
  };                                        \

EXPR_OP_NODE_CLASS(plus,+);
EXPR_OP_NODE_CLASS(minus,-);
EXPR_OP_NODE_CLASS(mult,*);
EXPR_OP_NODE_CLASS(div,/);

class atomic_constraint_node {
protected:
  shared_ptr<expr_tree_node> left;
  shared_ptr<expr_tree_node> right;
public :
  virtual bool is_satisfied() = 0;
  void set_left(shared_ptr<expr_tree_node> l) {
    left = l;
  }
  void set_right(shared_ptr<expr_tree_node> r) {
    right = r;
  }
};

#define ATOMIC_CONSTRAINT_NODE_CLASS(xxx,sym)         \
  class xxx##_node : public atomic_constraint_node {  \
    public:                                           \
      virtual bool is_satisfied() {                   \
        int l = left->compute();                      \
        int r = right->compute();                     \
        return l sym r;                               \
      }                                               \
  };                                                  \

ATOMIC_CONSTRAINT_NODE_CLASS(l,<);
ATOMIC_CONSTRAINT_NODE_CLASS(leq,<=);
ATOMIC_CONSTRAINT_NODE_CLASS(eq,==);
ATOMIC_CONSTRAINT_NODE_CLASS(geq,>=);
ATOMIC_CONSTRAINT_NODE_CLASS(g,>);

class builder {
  stack< shared_ptr<expr_tree_node> > st;
  shared_ptr<atomic_constraint_node> at_node;
public:
  void make_leaf(parser_context &pc){
    auto x = pc.collect_tokens();
    if (x.size() < 1) throw string("Error in collecting integer.");
    int v = atoi(x[x.size()-1].second.c_str());
    cout << v << endl;
    auto node = make_shared<expr_leaf_node>(v);
    st.push(node);
  }

  template<class T>
  void make_op(parser_context &pc) {
    cout << " inside make_op : ";
    cout << "right - " << st.top()->compute();
    auto r = st.top(); st.pop();
    cout << "; left - " << st.top()->compute() << endl;
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


  template<class T>
  void make_at(parser_context &pc) {
    cout << "Inside make_at " << endl;
    cout << " the size of st " << st.size() << endl;
    auto r = st.top(); st.pop();
    auto l = st.top(); st.pop();
    auto n = make_shared<T>();
    n->set_left(l);
    n->set_right(r);
    at_node = n;
  }

  shared_ptr<atomic_constraint_node> get_tree() {
    return at_node;
  }
};

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

}



#endif
