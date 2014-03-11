#ifndef __CONSTRAINT_HPP__
#define __CONSTRAINT_HPP__

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
    virtual void print() = 0;
    virtual AT_Constraint to_AT_Constraint(const CVList &cvl, const DVList &dvl) = 0;
  
    void set_left(std::shared_ptr<expr_tree_node> l);
    void set_right(std::shared_ptr<expr_tree_node> r);
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
    virtual void print() {                                              \
      left->print();                                                    \
      std::cout << #sym ;                                                 \
      right->print();                                                   \
    }                                                                   \
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
    bool eval(const DVList &dvl);
    void append_atomic_constraint(std::shared_ptr<atomic_constraint_node> at);
    Linear_Constraint to_Linear_Constraint(const CVList &cvl, const DVList &dvl);
    void print();
};

/**
   This class represents an assignment.
 */
class assignment {
public:
    std::string x;
    std::shared_ptr<expr_tree_node> expr;
    void print();
};

#endif
