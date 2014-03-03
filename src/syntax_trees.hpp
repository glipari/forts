#ifndef _SYNTAX_TREES_HPP_
#define _SYNTAX_TREES_HPP_

#include <string>
#include <stack>
#include <sstream>
#include <memory>

#include <tipa/tinyparser.hpp>

using namespace std;
using namespace tipa;

/**
   This class models a generic node in the syntax tree.  The only
   virtual method is eval() to evalulate an integer value for the
   tree. 

   @todo: other methods may be added later
 */
class expr_tree_node {
public : 
    virtual int eval() = 0;
    virtual ~expr_tree_node() {}
};

/**
   This class models a "operation" node, that is one between the four 
   arithmetic operations *, /, +, -

   It is possible to set the right and the left subtree.
   A node is "shared owner" of its subtree. Therefore, all nodes 
   should always be wrapped into a std::shared_ptr<> 
 */
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

/**
   A leaf node in the tree that represents a variable. 
 */
class expr_var_node : public expr_tree_node {
    string name;
    int value;
public:
    expr_var_node(const string &n) : name(n), value(0){}
    virtual int eval() { return value; }
};

/**
   A leaf node in the tree that represents an integer
 */
class expr_leaf_node : public expr_tree_node {
    int value;
public:
    expr_leaf_node(int v) : value(v){}
    virtual int eval() {return value;}
};

#define EXPR_OP_NODE_CLASS(xxx,sym) \
    class xxx##_node : public expr_op_node {	\
    public:					\
    virtual int eval() {			\
        int l = left->eval();            \
        int r = right->eval();           \
        return l sym r;                     \
      }                                     \
  };                                        \

EXPR_OP_NODE_CLASS(plus,+);
EXPR_OP_NODE_CLASS(minus,-);
EXPR_OP_NODE_CLASS(mult,*);
EXPR_OP_NODE_CLASS(div,/);


/**
   This class represents an atomic constraint An atomic constrains
   contains a left and a right subtree of expressions.
   It is possible to see if the constraint is satisfied for certain
   values of the variables.
 */
class atomic_constraint_node {
protected:
    shared_ptr<expr_tree_node> left;
    shared_ptr<expr_tree_node> right;
public :
    virtual bool eval() = 0;
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
      virtual bool eval() {                   \
        int l = left->eval();                      \
        int r = right->eval();                     \
        return l sym r;                               \
      }                                               \
  };                                                  \

ATOMIC_CONSTRAINT_NODE_CLASS(l,<);
ATOMIC_CONSTRAINT_NODE_CLASS(leq,<=);
ATOMIC_CONSTRAINT_NODE_CLASS(eq,==);
ATOMIC_CONSTRAINT_NODE_CLASS(geq,>=);
ATOMIC_CONSTRAINT_NODE_CLASS(g,>);


class constraint_node {
protected:
  vector< shared_ptr<atomic_constraint_node> > ats;
public:
  bool eval() {
    for (auto it = ats.begin(); it != ats.end(); it++) {
      if (!(*it)->eval()) {
        return false;
      }
    }
    return true;
  }
  void append_atomic_constraint(shared_ptr<atomic_constraint_node> at){
    ats.push_back(at);
  }
};

/**
   This is a function to build an atomic constraint from a simple
   string. 
 */
//shared_ptr<atomic_constraint_node> build_an_at_tree(string expr_input);
/**
   This is a function to build a constraint from a simple
   string. 
 */
shared_ptr<constraint_node> build_a_constraint_tree(string expr_input);

#endif
