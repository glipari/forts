#ifndef _SYNTAX_TREES_HPP_
#define _SYNTAX_TREES_HPP_

#include <string>
#include <stack>
#include <sstream>
#include <memory>

#include <tinyparser.hpp>

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

class builder {
  stack< shared_ptr<expr_tree_node> > st;
public:
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

  shared_ptr<expr_tree_node> get_tree() {
    return st.top();
  }
};

shared_ptr<expr_tree_node> build_an_expr_tree(string expr_input)
{
  rule expr, primary, term, op_plus, op_minus, op_mult, op_div,
       r_int, r_var;
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
  op_div   [std::bind(&builder::make_op<div_node>,           &b, _1)];

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

}




#endif
