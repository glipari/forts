#include "automaton.hpp"

using namespace std;

void Location::abstract_rates(const VariableList &cvl, 
						       const Valuations &dvl, 
						       vector<int> &v_rates) const
{
    int pos = 0;
    for ( auto it = cvl.begin(); it != cvl.end(); it++, pos++) {
        auto x = *it;
	    for ( auto iit = rates.begin(); iit != rates.end(); iit++) {
	        if (x == iit->get_var()) {
                int r = iit->eval(dvl);
                v_rates[pos] = r;
                break;
            }
        }
    }

}


Linear_Constraint Location::rates_to_Linear_Constraint(const VariableList &cvl, 
						       const Valuations &dvl, 
						       VariableList &lvars) const
{
    Linear_Constraint lc;
    for ( auto const &x : cvl) {
	//string x = it->name;
	for ( auto iit = rates.begin(); iit != rates.end(); iit++) {
	    if (x == iit->get_var()) {
		PPL::Variable v = get_ppl_variable(cvl, x);
		Linear_Expr le = iit->to_Linear_Expr(cvl, dvl);
		AT_Constraint atc = (v==le);
		lc.insert(atc);
		for (auto lt = lvars.begin(); lt != lvars.end(); lt++)
		    if ( x == *lt) {
			lvars.erase(lt);
			break;
		    }
		break;
	    }
	}
    }
    return lc;
}

Linear_Constraint Location::invariant_to_Linear_Constraint(const VariableList &cvl, 
							   const Valuations &dvl) const
{
    return invariant.to_Linear_Constraint(cvl, dvl);
}

void Location::print() const
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

// automaton::automaton() : my_index(0)
// {
// }


// Location::Location() : a_index(0)
// {
// }

Location::Location(bool b, const std::string &n, 
		   const constraint_node &inv,
		   const vector<Assignment> &rt, 
		   const vector<Edge> &ed) :
    aut(nullptr),
    bad(b),
    name(n),
    invariant(inv),
    rates(rt),
    outgoings(ed)
{
}

Location::Location(bool b, const std::string &n, 
		   const constraint_node &inv,
		   const constraint_node &f,
		   const vector<Edge> &ed) :
    aut(nullptr),
    bad(b),
    name(n),
    invariant(inv),
    flow(f),
    flow_flag(true),
    outgoings(ed)
{
}

/*void Location::set_automata_index(int a)
{
    a_index = a;
    for (auto &e : outgoings) e.set_automata_index(a);
    }*/

void Location::set_automaton(automaton &a) 
{
    aut = &a;
    for (auto &e : outgoings) e.set_src_location(*this);
}

automaton::automaton(const std::string &n,
		     const  std::vector<std::string> &lbls,
		     const std::vector<Location> locs
    ) :
    my_index(0),
    name(n),
    init_loc_name(""),
    labels(lbls),
    locations(locs)
{
    sort(labels.begin(), labels.end());
}

void automaton::set_index(int a)
{
    my_index = a;
    for (auto &l : locations) l.set_automaton(*this);
}

// TODO: change exception type
Location & automaton::get_location_by_name(std::string ln)
{
    for ( auto it = locations.begin(); it != locations.end(); it++)
	if ( it->get_name() == ln)
	    return *it;
    throw string("No location named ") + ln + string(" in automaton ") + get_name();
} 


void automaton::set_init_location(const std::string &init)
{ 
    Location &l = get_location_by_name(init); // checks that the location exists
    init_loc_name = init; 
}


void automaton::print() const 
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
bool automaton::check_consistency(const VariableList &cvl, const Valuations &dvl) const 
{
    for (auto it = locations.begin(); it != locations.end(); it++) {
	/** Check consistency of the invariant in each location. */
	it->invariant_to_Linear_Constraint(cvl, dvl);

	std::vector<Edge> outgoings = it->get_edges();
	for (auto iit = outgoings.begin(); iit != outgoings.end(); iit++){

	    /** Check consistency of the guard in each edge. */
	    //auto tt = iit->guard;
	    iit->guard_to_Linear_Constraint(cvl, dvl);

	    /** The destination location must exist. */
	    string dest = iit->get_dest();
	    bool dest_matched = false;
	    for ( auto tt = locations.begin(); tt != locations.end(); tt++)
		if ( dest == tt->get_name()) {
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
