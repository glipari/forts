#ifndef _SYNTAX_TREES_HPP_
#define _SYNTAX_TREES_HPP_

#include <expression.hpp>
#include <tipa/tinyparser.hpp>
using namespace std;
using namespace tipa;

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
    void print() {
      for (auto it = ats.begin(); it != ats.end(); it++) {
        if ( it != ats.begin())
          std::cout << "&" ;
        (*it)->print();
      }
    }
};

/**
   This is a function to build an atomic constraint from a simple
   string. 
 */
std::shared_ptr<constraint_node> build_a_constraint_tree(std::string expr_input);

std::shared_ptr<expr_tree_node> build_an_expr_tree(const std::string &expr_input);

class builder : public expr_builder {
    vector< shared_ptr<atomic_constraint_node> > at_nodes;
    shared_ptr<constraint_node> constraint;
public:
    builder () : expr_builder() { constraint = nullptr; }
    
    template<class T>
    void store_comp(parser_context &pc) {
	auto x = pc.collect_tokens();
	auto at_node = make_shared<T>(); 
        at_nodes.push_back(at_node);
    }
    
    shared_ptr<constraint_node> get_tree() {
	if ( constraint != nullptr) {
	    return constraint;
	}
	constraint = make_shared<constraint_node>();
	for ( auto it = at_nodes.rbegin(); it != at_nodes.rend(); it ++) {
	    //auto it = at_nodes.back();
	    auto r = st.top(); st.pop();
	    auto l = st.top(); st.pop();
	    (*it)->set_left(l);
	    (*it)->set_right(r);
	}

	for ( auto it = at_nodes.begin(); it != at_nodes.end(); it ++)
	    constraint->append_atomic_constraint(*it);
	return constraint;
    }
};

typedef builder constraint_builder;

rule prepare_constraint_rule(constraint_builder &cb);

#endif
