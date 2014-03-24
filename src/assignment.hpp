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
    
    Linear_Expr to_Linear_Expr(const CVList &cvl, const DVList &dvl) const;
    // Prints the value of this expression

    // evaluates the expression (mainly for debugging)
    int eval(const CVList &cvl) const;

    void print();
};


#endif 
