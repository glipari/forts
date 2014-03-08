#ifndef _SYNTAX_TREES_HPP_
#define _SYNTAX_TREES_HPP_

#include <expression.hpp>


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

#define ATOMIC_CONSTRAINT_NODE_CLASS(xxx,sym)				\
    class xxx##_node : public atomic_constraint_node {			\
    public:								\
    virtual bool eval(const DVList &dvl) {				\
        int l = left->eval(dvl);					\
        int r = right->eval(dvl);					\
        return l sym r;							\
    }									\
    virtual AT_Constraint to_AT_Constraint(const CVList &cvl, const DVList &dvl) { \
        Linear_Expr l = left->to_Linear_Expr(cvl, dvl);			\
        Linear_Expr r = right->to_Linear_Expr(cvl, dvl);		\
        AT_Constraint c;						\
        c = l sym r;							\
        return c;							\
    }									\
    };									\

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
    void append_atomic_constraint(std::shared_ptr<atomic_constraint_node> at) {
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
