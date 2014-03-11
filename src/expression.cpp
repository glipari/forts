#include "expression.hpp"

using namespace std;

bool expr_op_node::has_variable(const CVList &cvl) 
{
    return left->has_variable(cvl) || right->has_variable(cvl);
}

void expr_op_node::set_left(std::shared_ptr<expr_tree_node> l)
{
    left = l;
}

void expr_op_node::set_right(std::shared_ptr<expr_tree_node> r)
{
    right = r;
}

expr_var_node::expr_var_node(const std::string &n) : name(n) 
{}

int expr_var_node::eval(const DVList &dvl) 
{ 
    return var_2_val(name, dvl); 
}

bool expr_var_node::has_variable(const CVList &cvl) 
{
    return in_VList(name, cvl);
}

bool expr_var_node::check_linearity(const CVList &cvl) 
{
    return true;  
}

Linear_Expr expr_var_node::to_Linear_Expr(const CVList &cvl, const DVList &dvl) 
{
    Linear_Expr le;
    if ( in_VList(name, dvl) ) {
	le += var_2_val(name, dvl);
	return le;
    }
    PPL::Variable var = get_variable(name, cvl);
    le += var;
    return le;
}

void expr_var_node::print() 
{
    cout << name ;
}

expr_leaf_node::expr_leaf_node(int v) : value(v)
{}

int expr_leaf_node::eval(const DVList &dvl) 
{
    return value;
}

bool expr_leaf_node::has_variable(const CVList &cvl) 
{
    return false;
}

bool expr_leaf_node::check_linearity(const CVList &cvl) 
{
    return true;  
}

Linear_Expr expr_leaf_node::to_Linear_Expr(const CVList &cvl, const DVList &dvl) 
{
    Linear_Expr le;
    le += value;
    return le;
}

void expr_leaf_node::print() 
{
    cout << value ;
}

