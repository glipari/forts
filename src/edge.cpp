#include <iostream>

#include "edge.hpp"
#include "unique_index.hpp"
#include "automaton.hpp"

using namespace std;

Edge::Edge(const std::string &destination, 
	   const std::string &label,
	   const constraint_node &guard_constraint,
	   const std::vector<Assignment> &ass_list) :
    index(UniqueIndex::get_next_index()),
    a_index(0),
    aut(nullptr),
    src_location(nullptr),
    dest(destination),
    dst_location(nullptr),
    sync_label(label),
    guard(guard_constraint),
    assignments(ass_list)
{
}

PPL::Variables_Set Edge::get_assignment_vars(const VariableList &cvars) const
{
    PPL::Variables_Set vs;
    for (auto &a : assignments)
        if (contains<string>(cvars,a.get_var()))
	        vs.insert(get_ppl_variable(cvars, a.get_var()));
    return vs;
}


Linear_Constraint Edge::guard_to_Linear_Constraint(const VariableList &cvl, 
						   const Valuations &dvl) const
{
    return guard.to_Linear_Constraint(cvl, dvl);
}

Linear_Constraint Edge::ass_to_Linear_Constraint(const VariableList &cvl, 
						 Valuations &dvl) const
{
    Linear_Constraint lc;

	//string x = it->name;
	for ( auto iit = assignments.begin(); iit != assignments.end(); iit++) {
        /** If iit->get_var() is a discrete variable, update it here. */
        if (contains<string, int>(dvl, iit->get_var())) {
            int new_val = iit->eval(dvl);
            set_valuation(dvl, iit->get_var(), new_val);
            continue;
        }
        for ( auto const &x : cvl) {//it = cvl.begin(); it != cvl.end(); it++) {
	        if ( x == iit->get_var()) {
		        PPL::Variable v = get_ppl_variable(cvl, x);
		        Linear_Expr le = iit->to_Linear_Expr(cvl, dvl);
		        AT_Constraint atc = (v==le);
		        lc.insert(atc);
		        break;
	        }
	    }

    }
    return lc;
}

void Edge::print() const
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

void Edge::set_src_location(Location &l) 
{ 
    src_location = &l;
    aut = &(l.get_automaton()); 
    a_index = aut->get_index();
    
    dst_location = &aut->get_location_by_name(dest);
}


// void Edge::set_automata_index(int a)
// {
//     a_index = a;
// }

