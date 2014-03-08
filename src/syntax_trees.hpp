#ifndef _SYNTAX_TREES_HPP_
#define _SYNTAX_TREES_HPP_

#include <string>
#include <stack>
#include <sstream>
#include <memory>

#include <common.hpp>
#include <ppl_adapt.hpp>

//using namespace std;
//using namespace tipa;


/**
   This class models a generic node in the syntax tree.
 */
class expr_tree_node {
public : 
    virtual int eval(const DVList &dvl) = 0; 
    virtual bool has_variable(const CVList &cvl) = 0; 
    virtual bool check_linearity(const CVList &cvl) = 0; 
    virtual Linear_Expr to_Linear_Expr(const CVList &cvl, const DVList &dvl) = 0;
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
    std::shared_ptr<expr_tree_node> left;
    std::shared_ptr<expr_tree_node> right;
public:
    virtual bool has_variable(const CVList &cvl) {
      return left->has_variable(cvl) || right->has_variable(cvl);
    }
    void set_left(std::shared_ptr<expr_tree_node> l){
	left = l;
    }
    void set_right(std::shared_ptr<expr_tree_node> r){
	right = r;
    }
};

/**
   A leaf node in the tree that represents a variable. 
 */
class expr_var_node : public expr_tree_node {
    std::string name;
    int value;
public:
    virtual bool has_variable(const CVList &cvl) {
      return in_VList(name, cvl);
    }
    virtual bool check_linearity(const CVList &cvl) {
      return true;  
    }
    expr_var_node(const std::string &n) : name(n) {}
    virtual int eval(const DVList &dvl) { 
      return var_2_val(name, dvl); 
    }
    virtual Linear_Expr to_Linear_Expr(const CVList &cvl, const DVList &dvl) {
      Linear_Expr le;
      if ( in_VList(name, dvl) ) {
        le += var_2_val(name, dvl);
        return le;
      }
      PPL::Variable var = get_variable(name, cvl);
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
    virtual bool has_variable(const CVList &cvl) {
      return false;
    }
    virtual bool check_linearity(const CVList &cvl) {
      return true;  
    }
    expr_leaf_node(int v) : value(v){}
    virtual int eval(const DVList &dvl) {return value;}
    virtual Linear_Expr to_Linear_Expr(const CVList &cvl, const DVList &dvl) {
      Linear_Expr le;
      le += value;
      return le;
    }
};


/** 
    These nodes represent operations
*/
#define EXPR_OP_NODE(xxx,sym) \
    class expr_##xxx : public expr_op_node {	\
    public:					\
    virtual int eval(const DVList &dvl) {			\
        int l = left->eval(dvl);            \
        int r = right->eval(dvl);           \
        return l sym r;                     \
      }                                     \
  };                                        \

EXPR_OP_NODE(mult,*);
//EXPR_OP_NODE(div,/);
EXPR_OP_NODE(plus,+);
EXPR_OP_NODE(minus,-);

#define NNLinear_EXPR_OP_NODE_CLASS(xxx,sym) \
    class xxx##_node : public expr_##xxx {	\
    public:					\
      virtual bool check_linearity(const CVList &cvl) { \
        bool r = right->has_variable(cvl);   \
        bool l = left->has_variable(cvl);   \
        if ( r && l)\
          return false;\
        return right->check_linearity(cvl) && left->check_linearity(cvl); \
      }\
      virtual Linear_Expr to_Linear_Expr(const CVList &cvl, const DVList &dvl) { \
        if ( !check_linearity(cvl)) \
          throw ("Not a linear expression");\
        Linear_Expr le; \
        bool l = left->has_variable(cvl); \
        std::cout << "left is " << l << std::endl;	\
        if ( l )  \
          return left->to_Linear_Expr(cvl,dvl) sym right->eval(dvl); \
        else  \
          return left->eval(dvl) sym right->to_Linear_Expr(cvl,dvl); \
      }   \
  };                                        \

#define LINEAR_EXPR_OP_NODE_CLASS(xxx,sym) \
    class xxx##_node : public expr_##xxx {	\
    public:					\
      virtual bool check_linearity(const CVList &cvl) { \
        bool r = right->check_linearity(cvl);   \
        bool l = left->check_linearity(cvl);   \
        return r && l;\
      }\
      virtual Linear_Expr to_Linear_Expr(const CVList &cvl, const DVList &dvl) { \
        Linear_Expr le , l , r; \
        l = left->to_Linear_Expr(cvl,dvl);  \
        r = right->to_Linear_Expr(cvl,dvl); \
        return l sym r; \
      }   \
  };                                        \

LINEAR_EXPR_OP_NODE_CLASS(plus,+);
LINEAR_EXPR_OP_NODE_CLASS(minus,-);
NNLinear_EXPR_OP_NODE_CLASS(mult,*);


/**
   This class represents an atomic constraint An atomic constrains
   contains a left and a right subtree of expressions.
   It is possible to see if the constraint is satisfied for certain
   values of the variables.
 */
class atomic_constraint_node {
protected:
    std::shared_ptr<expr_tree_node> left;
    std::shared_ptr<expr_tree_node> right;
public :
    virtual bool eval(const DVList &dvl) = 0;
    virtual AT_Constraint to_AT_Constraint(const CVList &cvl, const DVList &dvl) = 0; 
  
    void set_left(std::shared_ptr<expr_tree_node> l) {
	left = l;
    }
    void set_right(std::shared_ptr<expr_tree_node> r) {
	right = r;
    }
};

#define ATOMIC_CONSTRAINT_NODE_CLASS(xxx,sym)         \
  class xxx##_node : public atomic_constraint_node {  \
    public:                                           \
      virtual bool eval(const DVList &dvl) {                   \
        int l = left->eval(dvl);                      \
        int r = right->eval(dvl);                     \
        return l sym r;                               \
      }                                               \
      virtual AT_Constraint to_AT_Constraint(const CVList &cvl, const DVList &dvl) { \
        Linear_Expr l = left->to_Linear_Expr(cvl, dvl);               \
        Linear_Expr r = right->to_Linear_Expr(cvl, dvl);             \
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
    std::vector< std::shared_ptr<atomic_constraint_node> > ats;
public:
  bool eval(const DVList &dvl) {
    for (auto it = ats.begin(); it != ats.end(); it++) {
      if (!(*it)->eval(dvl)) {
        return false;
      }
    }
    return true;
  }
  void append_atomic_constraint(std::shared_ptr<atomic_constraint_node> at){
    ats.push_back(at);
  }

  Linear_Constraint to_Linear_Constraint(const CVList &cvl, const DVList &dvl) {
    Linear_Constraint c;
    for ( auto it = ats.begin(); it != ats.end(); it ++)
      c.insert((*it)->to_AT_Constraint(cvl, dvl));
    return c;
  }
};

/**
   This is a function to build an atomic constraint from a simple
   string. 
 */
std::shared_ptr<constraint_node> build_a_constraint_tree(std::string expr_input);

#endif
