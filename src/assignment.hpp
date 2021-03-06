#ifndef __ASSIGNMENT_HPP__
#define __ASSIGNMENT_HPP__

#include <expression.hpp>

/**
   This class represents an assignment, in the form of 

   <var_name>' = <expression> 
   
 */
class Assignment {
    std::string x;
    std::shared_ptr<const expr_tree_node> expr;
public:
    // Constructor
    Assignment(const std::string &var, std::shared_ptr<const expr_tree_node> &ex);

    std::string get_var() const;
    
    Linear_Expr to_Linear_Expr(const VariableList &cvl, const Valuations &dvl) const;
    // Prints the value of this expression

    // evaluates the expression (mainly for debugging)
    int eval(const Valuations &cvl) const;

    void print() const;

    bool has_variable(const std::string &v) const;
};


#endif 
