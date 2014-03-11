#ifndef __EXPRESSION_PARSER__
#define __EXPRESSION_PARSER__

#include <tipa/tinyparser.hpp>
#include <expression.hpp>

/**
   This class is a "builder" that construct the expression tree
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
   This function parses an expression from a string. It returns an
   expression tree. This function uses the "prepare_expr_rule" to
   prepare the grammar to parse the string.
 */
std::shared_ptr<expr_tree_node> build_expression(const std::string &expr_input);


#endif
