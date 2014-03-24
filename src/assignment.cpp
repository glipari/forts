#include "assignment.hpp"

Assignment::Assignment(const std::string &var, std::shared_ptr<const expr_tree_node> &ex)
    : x(var), expr(ex)
{
}

std::string Assignment::get_var() const { return x; }

Linear_Expr Assignment::to_Linear_Expr(const CVList &cvl, const DVList &dvl) const
{
    return expr->to_Linear_Expr(cvl, dvl);
}

int Assignment::eval(const CVList &cvl) const
{
    return expr->eval(cvl);
}


void Assignment::print() 
{
    std::cout << x;
    std::cout << "'=";
    expr->print();
}
