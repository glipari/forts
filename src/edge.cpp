#include <iostream>

#include "edge.hpp"
#include "unique_index.hpp"

using namespace std;

edge::edge(const std::string &destination, 
	   const std::string &label,
	   const constraint_node &guard_constraint,
	   const std::vector<Assignment> &ass_list) :
    index(UniqueIndex::get_next_index()),
    a_index(0),
    dest(destination),
    sync_label(label),
    guard(guard_constraint),
    assignments(ass_list)
{
}

PPL::Variables_Set edge::get_assignment_vars(const CVList &cvars) const
{
    PPL::Variables_Set vs;
    for (auto &a : assignments)
	vs.insert(get_variable(a.get_var(), cvars));
    return vs;
}


Linear_Constraint edge::guard_to_Linear_Constraint(const CVList &cvl, const DVList &dvl) const
{
    return guard.to_Linear_Constraint(cvl, dvl);
}

Linear_Constraint edge::ass_to_Linear_Constraint(const CVList &cvl, const DVList &dvl) const
{
    Linear_Constraint lc;

    for ( auto it = cvl.begin(); it != cvl.end(); it++) {
	string x = it->name;
	for ( auto iit = assignments.begin(); iit != assignments.end(); iit++) {
	    if ( x == iit->get_var()) {
		PPL::Variable v = get_variable(x, cvl);
		Linear_Expr le = iit->to_Linear_Expr(cvl, dvl);
		AT_Constraint atc = (v==le);
		lc.insert(atc);
		break;
	    }
	}

    }
    return lc;
}

void edge::print() const
{
    std::cout << "when ";
    guard.print();
    cout << " sync " << sync_label;
    std::cout << " do {";
    bool first = true;
    for ( auto & a : assignments ) {//it = assignments.begin(); it != assignments.end(); it++) {
	if (!first) cout << ", ";
	first = false;
	a.print();
    }
    std::cout << " }";
    std::cout << " goto " << dest << ";\n";
}


void edge::set_automata_index(int a)
{
    a_index = a;
}

