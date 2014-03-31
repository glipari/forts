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
    expr_tree_node(const expr_tree_node &e) = delete;
    expr_tree_node& operator=(const expr_tree_node &e) = delete;

    expr_tree_node() {}

    virtual int eval(const Valuations &dvl) const = 0; 
    virtual bool has_variable(const VariableList &cvl) const = 0; 
    virtual bool check_linearity(const VariableList &cvl) const = 0; 
    virtual Linear_Expr to_Linear_Expr(const VariableList &cvl, const Valuations &dvl) const = 0;
    virtual void print() const = 0;

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
    std::shared_ptr<const expr_tree_node> left;
    std::shared_ptr<const expr_tree_node> right;

public:
    expr_op_node(std::shared_ptr<const expr_tree_node> &l, std::shared_ptr<const expr_tree_node> &r) :
	left(l), right(r) {}

    virtual bool has_variable(const VariableList &cvl) const;
    // void set_left(std::shared_ptr<expr_tree_node> l);
    // void set_right(std::shared_ptr<expr_tree_node> r);
};

/**
   A leaf node in the tree that represents a variable. 
 */
class expr_var_node : public expr_tree_node {
    std::string name;
    int value;
public:

    expr_var_node(const std::string &n);

    virtual int eval(const Valuations &dvl) const ;
    virtual bool has_variable(const VariableList &cvl) const ;
    virtual bool check_linearity(const VariableList &cvl) const ;
    virtual Linear_Expr to_Linear_Expr(const VariableList &cvl, const Valuations &dvl) const;
    virtual void print() const;
};

/**
   A leaf node in the tree that represents an integer
 */
class expr_leaf_node : public expr_tree_node {
    int value;
public:
    expr_leaf_node(int v);

    virtual int eval(const Valuations &dvl) const;
    virtual bool has_variable(const VariableList &cvl) const;
    virtual bool check_linearity(const VariableList &cvl) const;
    virtual Linear_Expr to_Linear_Expr(const VariableList &cvl, const Valuations &dvl) const;
    virtual void print() const;
};

#define NNLinear_EXPR_OP_NODE_CLASS(xxx,sym)				\
    class xxx##_node : public expr_op_node {				\
    public:								\
    xxx##_node(std::shared_ptr<const expr_tree_node> &l, std::shared_ptr<const expr_tree_node> &r) : \
        expr_op_node(l,r) {}						\
    virtual int eval(const Valuations &dvl) const {				\
        int l = left->eval(dvl);					\
        int r = right->eval(dvl);					\
        return l sym r;							\
    }									\
									\
    virtual bool check_linearity(const VariableList &cvl) const {		\
        bool r = right->has_variable(cvl);				\
        bool l = left->has_variable(cvl);				\
        if (r && l)							\
	    return false;						\
        return right->check_linearity(cvl) && left->check_linearity(cvl); \
    }									\
									\
    virtual Linear_Expr to_Linear_Expr(const VariableList &cvl, const Valuations &dvl) const { \
	if ( !check_linearity(cvl))					\
	    throw ("Not a linear expression");				\
	Linear_Expr le;							\
	bool l = left->has_variable(cvl);				\
	if (l)								\
	    return left->to_Linear_Expr(cvl,dvl) sym right->eval(dvl);	\
	else								\
	    return left->eval(dvl) sym right->to_Linear_Expr(cvl, dvl); \
    }									\
    virtual void print() const {					\
      left->print();                                                    \
      std::cout << #sym;                                                 \
      right->print();                                                   \
    }                                                                   \
    };                                        

#define LINEAR_EXPR_OP_NODE_CLASS(xxx,sym)				\
    class xxx##_node : public expr_op_node {				\
    public:								\
    xxx##_node(std::shared_ptr<const expr_tree_node> &l, std::shared_ptr<const expr_tree_node> &r) : \
        expr_op_node(l,r) {}						\
									\
    virtual int eval(const Valuations &dvl) const {				\
        int l = left->eval(dvl);					\
        int r = right->eval(dvl);					\
        return l sym r;							\
    }									\
									\
    virtual bool check_linearity(const VariableList &cvl) const {		\
        bool r = right->check_linearity(cvl);				\
        bool l = left->check_linearity(cvl);				\
        return r && l;							\
    }									\
    virtual Linear_Expr to_Linear_Expr(const VariableList &cvl, const Valuations &dvl) const { \
	Linear_Expr le , l , r;						\
	l = left->to_Linear_Expr(cvl,dvl);				\
	r = right->to_Linear_Expr(cvl,dvl);				\
	return l sym r;							\
    }									\
    virtual void print() const {					\
	left->print();							\
	std::cout << #sym ;						\
	    right->print();						\
    }                                                                   \
    };                                        

LINEAR_EXPR_OP_NODE_CLASS(plus,+);
LINEAR_EXPR_OP_NODE_CLASS(minus,-);
NNLinear_EXPR_OP_NODE_CLASS(mult,*);

#endif
