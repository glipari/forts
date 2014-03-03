#ifndef _SYNTAX_TREES_HPP_
#define _SYNTAX_TREES_HPP_

#include <string>
#include <stack>
#include <sstream>
#include <memory>

#include <tipa/tinyparser.hpp>

#include "common.hpp"

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
    virtual int eval(const DVList_ptr vt = nullptr) = 0; 
    virtual Linear_Expr to_Linear_Expr(const CVList_ptr cv, const DVList_ptr vt = nullptr) = 0;
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
    virtual int eval(const DVList_ptr vt = nullptr) { 
      if ( vt == nullptr)
        return 0;
      return var_2_val(name, vt); 
    }
    virtual Linear_Expr to_Linear_Expr(const CVList_ptr cv, const DVList_ptr vt = nullptr) {
      Linear_Expr le;
      if ( in_DVList(name, vt) ) {
        le += var_2_val(name, vt);
        return le;
      }
      Variable var = get_variable(name, cv);
      le += var;
      return le;
    }
};

/**
   A leaf node in the tree that represents an integer
 */
class expr_leaf_node : public expr_tree_node {
    int value;
public:
    expr_leaf_node(int v) : value(v){}
    virtual int eval(const DVList_ptr vt = nullptr) {return value;}
    virtual Linear_Expr to_Linear_Expr(const CVList_ptr cv, const DVList_ptr vt = nullptr) {
      Linear_Expr le;
      le += value;
      return le;
    }
};

#define CONST_EXPR_OP_NODE_CLASS(xxx,sym) \
    class xxx##_node : public expr_op_node {	\
    public:					\
    virtual int eval(const DVList_ptr vt = nullptr) {			\
        int l = left->eval(vt);            \
        int r = right->eval(vt);           \
        return l sym r;                     \
      }                                     \
      virtual Linear_Expr to_Linear_Expr(const CVList_ptr cv, const DVList_ptr vt = nullptr) { \
        Linear_Expr le; \
        int l = left->eval(vt);  \
        int r = right->eval(vt); \
        le += l sym r;   \
        return le; \
      }   \
  };                                        \

#define LINEAR_EXPR_OP_NODE_CLASS(xxx,sym) \
    class xxx##_node : public expr_op_node {	\
    public:					\
    virtual int eval(const DVList_ptr vt = nullptr) {			\
        int l = left->eval(vt);            \
        int r = right->eval(vt);           \
        return l sym r;                     \
      }                                     \
      virtual Linear_Expr to_Linear_Expr(const CVList_ptr cv, const DVList_ptr vt = nullptr) { \
        Linear_Expr le , l , r; \
        l = left->to_Linear_Expr(cv,vt);  \
        r = right->to_Linear_Expr(cv,vt); \
        return l sym r; \
      }   \
  };                                        \

LINEAR_EXPR_OP_NODE_CLASS(plus,+);
LINEAR_EXPR_OP_NODE_CLASS(minus,-);
CONST_EXPR_OP_NODE_CLASS(mult,*);
CONST_EXPR_OP_NODE_CLASS(div,/);


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
    virtual bool eval(const DVList_ptr vt = nullptr) = 0;
    virtual AT_Constraint to_AT_Constraint(const CVList_ptr cv, const DVList_ptr vt = nullptr) = 0; 
  
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
      virtual bool eval(const DVList_ptr vt = nullptr) {                   \
        int l = left->eval(vt);                      \
        int r = right->eval(vt);                     \
        return l sym r;                               \
      }                                               \
      virtual AT_Constraint to_AT_Constraint(const CVList_ptr cv, const DVList_ptr vt = nullptr) { \
        Linear_Expr l = left->to_Linear_Expr(cv, vt);               \
        Linear_Expr r = right->to_Linear_Expr(cv, vt);             \
        AT_Constraint c;  \
        c = l sym r;                     \
        return c;                                     \
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
  bool eval(const DVList_ptr vtp = nullptr) {
    for (auto it = ats.begin(); it != ats.end(); it++) {
      if (!(*it)->eval(vtp)) {
        return false;
      }
    }
    return true;
  }
  void append_atomic_constraint(shared_ptr<atomic_constraint_node> at){
    ats.push_back(at);
  }

  Linear_Constraint to_Linear_Constraint(const CVList_ptr cvlp, const DVList_ptr vtp=nullptr) {
    Linear_Constraint c;
    for ( auto it = ats.begin(); it != ats.end(); it ++)
      c.insert((*it)->to_AT_Constraint(cvlp, vtp));
    return c;
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
