#ifndef _EXPRESSION_HPP_
#define _EXPRESSION_HPP_

#include <string>
#include <stack>
#include <memory>

#include <common.hpp>
#include <ppl_adapt.hpp>
#include <tipa/tinyparser.hpp>

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
    };                                        

LINEAR_EXPR_OP_NODE_CLASS(plus,+);
LINEAR_EXPR_OP_NODE_CLASS(minus,-);
NNLinear_EXPR_OP_NODE_CLASS(mult,*);

/**
   This object is a "builder" that construct the expression tree
   incrementally. The function members are invoked by an expression
   parser (see prepare_expr_rule() below.
 */
class expr_builder {
protected:
    std::stack< std::shared_ptr<expr_tree_node> > st;
public:
    expr_builder();
    void make_leaf(tipa::parser_context &pc);
    
    template<class T>
    void make_op(tipa::parser_context &pc) {
	auto r = st.top(); st.pop();
	auto l = st.top(); st.pop();
	auto n = std::make_shared<T>();
	n->set_left(l);
	n->set_right(r);
	st.push(n);
    }
    
    void make_var(tipa::parser_context &pc);
    int get_size();
    std::shared_ptr<expr_tree_node> get_tree();
};



/** 
    This function prepares a set of rules for parsing 
    an expression. It requires a "expr_builder" object, where the 
    expression tree is going to be built
*/
tipa::rule prepare_expr_rule(expr_builder &b);

/**
   This function parsers an expression from a string. It returns an
   expression tree. This function uses the "prepare_expr_rule" to
   prepare the grammar to parse the string.
 */
std::shared_ptr<expr_tree_node> build_expression(const std::string &expr_input);

#endif
