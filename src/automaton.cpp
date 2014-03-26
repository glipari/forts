#include "automaton.hpp"

using namespace std;


Linear_Constraint location::rates_to_Linear_Constraint(const CVList &cvl, const DVList &dvl, CVList &lvars)
{
    Linear_Constraint lc;
    for ( auto it = cvl.begin(); it != cvl.end(); it++) {
	string x = it->name;
	for ( auto iit = rates.begin(); iit != rates.end(); iit++) {
	    if ( x == iit->get_var()) {
		PPL::Variable v = get_variable(x, cvl);
		Linear_Expr le = iit->to_Linear_Expr(cvl, dvl);
		AT_Constraint atc = (v==le);
		lc.insert(atc);
		for ( auto lt = lvars.begin(); lt != lvars.end(); lt++)
		    if ( x==lt->name) {
			lvars.erase(lt);
			break;
		    }
		break;
	    }
	}

    }
    return lc;
}

Linear_Constraint location::invariant_to_Linear_Constraint(const CVList &cvl, const DVList &dvl)
{
    return invariant.to_Linear_Constraint(cvl, dvl);
}


void location::print() 
{
    std::cout << "loc " << name << ": while ";
    invariant.print();
    std::cout << " wait {";
    for ( auto it = rates.begin(); it != rates.end(); it++) {
	if ( it != rates.begin())
	    cout << ", ";
	it->print();
    }
    std::cout << " }\n";
    for (auto it = outgoings.begin(); it != outgoings.end(); it++)
	it->print();
}

automaton::automaton() : my_index(0)
{
}


location::location() : a_index(0)
{
}


void location::set_automata_index(int a)
{
    a_index = a;
    for (auto &e : outgoings) e.set_automata_index(a);
}

void automaton::set_index(int a)
{
    my_index = a;
    for (auto &l : locations) l.set_automata_index(a);
}


location & automaton::get_location(std::string ln)
{
    for ( auto it = locations.begin(); it != locations.end(); it++)
	if ( it->name == ln)
	    return *it;
    throw string("No location named ") + ln;
} 

void automaton::print()
{
    std::cout << "automaton " << name << std::endl;
    std::cout << "sync: ";
    for ( auto it = labels.begin(); it != labels.end(); it++) {
	if ( it != labels.begin())
	    std::cout << ", ";
	std::cout << *it ;
    }
    std:: cout << ";" << std::endl;
    for ( auto it = locations.begin(); it != locations.end(); it++) {
	it->print();
	std::cout << std::endl;
    }
    std:: cout << "end" << std::endl;
}


// TODO: change throw to something different than string
bool automaton::check_consistency(const CVList &cvl, const DVList &dvl)
{
    for (auto it = locations.begin(); it != locations.end(); it++) {
	/** Check consistency of the invariant in each location. */
	it->invariant.to_Linear_Constraint(cvl, dvl);

	for (auto iit = it->outgoings.begin(); iit != it->outgoings.end(); iit++){

	    /** Check consistency of the guard in each edge. */
	    //auto tt = iit->guard;
	    iit->guard_to_Linear_Constraint(cvl, dvl);

	    /** The destination location must exist. */
	    string dest = iit->get_dest();
	    bool dest_matched = false;
	    for ( auto tt = locations.begin(); tt != locations.end(); tt++)
		if ( dest == tt->name) {
		    dest_matched = true;
		    break;
		}
	    if (!dest_matched)
		throw string("The destination \"") + dest + string("\" does not exist.");

	    /** The sync lab must exist. */
	    string sync_label = iit->get_label();
	    if ( sync_label == "")
		continue;
	    bool lab_matched = false;
	    for ( auto tt = labels.begin(); tt != labels.end(); tt++)
		if ( sync_label == *tt) {
		    lab_matched = true;
		    break;
		}
	    if ( !lab_matched)
		throw string("The sync label \"") + sync_label + string("\" does not exist.");

	}
    }
    return true;
}
