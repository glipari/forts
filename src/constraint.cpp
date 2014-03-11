#include <constraint.hpp>

void atomic_constraint_node::set_left(std::shared_ptr<expr_tree_node> l) 
{
    left = l;
}

void atomic_constraint_node::set_right(std::shared_ptr<expr_tree_node> r) 
{
    right = r;
}


bool constraint_node::eval(const DVList &dvl) 
{
    for (auto it = ats.begin(); it != ats.end(); it++) {
	if (!(*it)->eval(dvl)) {
	    return false;
	}
    }
    return true;
}

void constraint_node::append_atomic_constraint(std::shared_ptr<atomic_constraint_node> at) 
{
    ats.push_back(at);
}

Linear_Constraint constraint_node::to_Linear_Constraint(const CVList &cvl, const DVList &dvl) 
{
    Linear_Constraint c;
    for ( auto it = ats.begin(); it != ats.end(); it ++)
	c.insert((*it)->to_AT_Constraint(cvl, dvl));
    return c;
}

void constraint_node::print() 
{
    for (auto it = ats.begin(); it != ats.end(); it++) {
        if ( it != ats.begin())
	    std::cout << "&" ;
        (*it)->print();
    }
}

void assignment::print() 
{
    std::cout << x;
    std::cout << "'=";
    expr->print();
}

