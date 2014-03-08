#ifndef _EXPRESSION_HPP_
#define _EXPRESSION_HPP_

#include <string>
//#include <stack>
#include <sstream>
#include <memory>

#include <common.hpp>
#include <ppl_adapt.hpp>


/*#include "syntax_trees.hpp"

class expression {
    std::shared_ptr<expr_tree_node> root; 
public:
    expression (): root(nullptr) {}
    };*/


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
#define EXPR_OP_NODE(xxx,sym)			\
    class expr_##xxx : public expr_op_node {	\
    public:					\
    virtual int eval(const DVList &dvl) {	\
        int l = left->eval(dvl);		\
        int r = right->eval(dvl);		\
        return l sym r;				\
    }						\
    };						\

EXPR_OP_NODE(mult,*);
EXPR_OP_NODE(plus,+);
EXPR_OP_NODE(minus,-);

std::shared_ptr<expr_tree_node> build_expression(const std::string &expr_input);

#endif
