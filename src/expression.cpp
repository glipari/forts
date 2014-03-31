#include "expression.hpp"

using namespace std;

bool expr_op_node::has_variable(const VariableList &cvl) const
{
    return left->has_variable(cvl) || right->has_variable(cvl);
}

expr_var_node::expr_var_node(const std::string &n) : name(n) 
{}

int expr_var_node::eval(const Valuations &dvl) const
{ 
    return get_valuation(dvl, name); 
}

bool expr_var_node::has_variable(const VariableList &cvl) const 
{
    return contains(cvl, name);
}

bool expr_var_node::check_linearity(const VariableList &cvl) const 
{
    return true;  
}

Linear_Expr expr_var_node::to_Linear_Expr(const VariableList &cvl, 
					  const Valuations &dvl) const
{
    Linear_Expr le;
    if (contains(dvl, name) ) {
	le += get_valuation(dvl, name);
	return le;
    }
    PPL::Variable var = get_ppl_variable(cvl, name);
    le += var;
    return le;
}

void expr_var_node::print() const 
{
    cout << name ;
}

expr_leaf_node::expr_leaf_node(int v) : value(v)
{}

int expr_leaf_node::eval(const Valuations &dvl) const 
{
    return value;
}

bool expr_leaf_node::has_variable(const VariableList &cvl) const
{
    return false;
}

bool expr_leaf_node::check_linearity(const VariableList &cvl) const
{
    return true;  
}

Linear_Expr expr_leaf_node::to_Linear_Expr(const VariableList &cvl, const Valuations &dvl) const
{
    Linear_Expr le;
    le += value;
    return le;
}

void expr_leaf_node::print() const
{
    cout << value ;
}

