#ifndef _EXPRESSION_HPP_
#define _EXPRESSION_HPP_

#include <string>
#include <stack>
#include <memory>

#include <common.hpp>
#include <ppl_adapt.hpp>

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
    virtual void print() = 0;
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
    virtual bool has_variable(const CVList &cvl);
    void set_left(std::shared_ptr<expr_tree_node> l);
    void set_right(std::shared_ptr<expr_tree_node> r);
};

/**
   A leaf node in the tree that represents a variable. 
 */
class expr_var_node : public expr_tree_node {
    std::string name;
    int value;
public:
    expr_var_node(const std::string &n);
    virtual int eval(const DVList &dvl);
    virtual bool has_variable(const CVList &cvl);
    virtual bool check_linearity(const CVList &cvl);
    virtual Linear_Expr to_Linear_Expr(const CVList &cvl, const DVList &dvl);
    virtual void print();
};

/**
   A leaf node in the tree that represents an integer
 */
class expr_leaf_node : public expr_tree_node {
    int value;
public:
    expr_leaf_node(int v);

    virtual int eval(const DVList &dvl);
    virtual bool has_variable(const CVList &cvl);
    virtual bool check_linearity(const CVList &cvl);
    virtual Linear_Expr to_Linear_Expr(const CVList &cvl, const DVList &dvl);
    virtual void print();
};

#define NNLinear_EXPR_OP_NODE_CLASS(xxx,sym)				\
    class xxx##_node : public expr_op_node {				\
    public:								\
									\
    virtual int eval(const DVList &dvl) {				\
        int l = left->eval(dvl);					\
        int r = right->eval(dvl);					\
        return l sym r;							\
    }									\
									\
    virtual bool check_linearity(const CVList &cvl) {			\
        bool r = right->has_variable(cvl);				\
        bool l = left->has_variable(cvl);				\
        if (r && l)							\
	    return false;						\
        return right->check_linearity(cvl) && left->check_linearity(cvl); \
    }									\
									\
    virtual Linear_Expr to_Linear_Expr(const CVList &cvl, const DVList &dvl) { \
	if ( !check_linearity(cvl))					\
	    throw ("Not a linear expression");				\
	Linear_Expr le;							\
	bool l = left->has_variable(cvl);				\
	std::cout << "left is " << l << std::endl;			\
	if (l)								\
	    return left->to_Linear_Expr(cvl,dvl) sym right->eval(dvl);	\
	else								\
	    return left->eval(dvl) sym right->to_Linear_Expr(cvl, dvl); \
    }									\
    virtual void print() {                                              \
      left->print();                                                    \
      std::cout << #sym;                                                 \
      right->print();                                                   \
    }                                                                   \
    };                                        

#define LINEAR_EXPR_OP_NODE_CLASS(xxx,sym)				\
    class xxx##_node : public expr_op_node {				\
    public:								\
									\
    virtual int eval(const DVList &dvl) {				\
        int l = left->eval(dvl);					\
        int r = right->eval(dvl);					\
        return l sym r;							\
    }									\
									\
    virtual bool check_linearity(const CVList &cvl) {			\
        bool r = right->check_linearity(cvl);				\
        bool l = left->check_linearity(cvl);				\
        return r && l;							\
    }									\
    virtual Linear_Expr to_Linear_Expr(const CVList &cvl, const DVList &dvl) { \
	Linear_Expr le , l , r;						\
	l = left->to_Linear_Expr(cvl,dvl);				\
	r = right->to_Linear_Expr(cvl,dvl);				\
	return l sym r;							\
    }									\
    virtual void print() {                                              \
      left->print();                                                    \
      std::cout << #sym ;                                                 \
      right->print();                                                   \
    }                                                                   \
    };                                        

LINEAR_EXPR_OP_NODE_CLASS(plus,+);
LINEAR_EXPR_OP_NODE_CLASS(minus,-);
NNLinear_EXPR_OP_NODE_CLASS(mult,*);

#endif
