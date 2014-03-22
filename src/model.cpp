#include <iostream>
#include <algorithm>

#include <expression.hpp>
#include <model.hpp>

using namespace std;
using namespace Parma_Polyhedra_Library::IO_Operators;

PPL::C_Polyhedron model::get_invariant_cvx(sstate &ss)
{
    PPL::C_Polyhedron invariant_cvx(cvars.size());
    for ( auto it = automata.begin(); it != automata.end(); it++){
	Linear_Constraint lc;
	string ln = ss.loc_names[it-automata.begin()];
	location &l = it->get_location(ln);

	invariant_cvx.add_constraints(l.invariant_to_Linear_Constraint(cvars, dvars));
    }
    return invariant_cvx;
}

void model::continuous_step(sstate &ss)
{
    // 1) To do time_elapse_assign
    PPL::C_Polyhedron rates_cvx(cvars.size());
    PPL::C_Polyhedron invariant_cvx(cvars.size());
    CVList lvars = cvars;
    for ( auto it = automata.begin(); it != automata.end(); it++){
	Linear_Constraint lc;
	string ln = ss.loc_names[it-automata.begin()];
	location &l = it->get_location(ln);

	invariant_cvx.add_constraints(l.invariant_to_Linear_Constraint(cvars, dvars));
	rates_cvx.add_constraints(l.rates_to_Linear_Constraint(cvars, dvars, lvars));
        //cout << "rates : " << rates_cvx << endl;
        //cout << "invariant : " << invariant_cvx << endl;
    
    }

    // If a variable's rate is not specified in the location, it's assumed to be 1
    for ( auto it = lvars.begin(); it != lvars.end(); it++) {
      PPL::Variable v = get_variable(it->name, cvars);
      Linear_Expr le;
      le += 1;
      AT_Constraint atc = (v==le);
      rates_cvx.add_constraint(atc);
    }
    ss.cvx.time_elapse_assign(rates_cvx);
    // 2) intersect with invariant
    ss.cvx.intersection_assign(invariant_cvx);
}

void model::discrete_step(sstate &ss, const vector<edge> &edges)
{
    //ss.loc_names.clear();
    PPL::C_Polyhedron guard_cvx(cvars.size());
    PPL::C_Polyhedron ass_cvx(cvars.size());
    Variables_Set vs;

    for (auto &e : edges) {
	ss.loc_names[e.a_index] = e.dest;
	guard_cvx.add_constraints(e.guard_to_Linear_Constraint(cvars, dvars));
	for (auto &a : e.assignments)
	    vs.insert(get_variable(a.x, cvars));
	ass_cvx.add_constraints(e.ass_to_Linear_Constraint(cvars, dvars));
    }
    ss.cvx.intersection_assign(guard_cvx);
    ss.cvx.unconstrain(vs);
    ss.cvx.intersection_assign(ass_cvx);
    ss.cvx.intersection_assign(get_invariant_cvx(ss));
}


void combine(vector<Combined_edge> &edge_groups, const location &l, 
	     const vector<string> new_labels,
	     bool first) 
{
    if (first) {
	for (auto iit = l.outgoings.begin(); iit != l.outgoings.end(); iit++) {
            Combined_edge egroup;
            egroup.edges.push_back(*iit);
            //if ( iit->sync_label != "") {
              egroup.sync_label = (iit->sync_label);
              egroup.sync_set = new_labels;
            //}
            edge_groups.push_back(egroup);
	}
	return;
    } 
    vector<Combined_edge> copy = edge_groups;
    edge_groups.clear();
    // to combine every outgoing from "l" with every "edge group"
    for ( auto &egroup : copy) {
      for ( auto it = l.outgoings.begin(); it != l.outgoings.end(); it++) {
        vector<Combined_edge> com = egroup.combine(*it, new_labels);
        for ( auto &eg : com)
          if ( not contains(edge_groups, eg))
            edge_groups.push_back(eg);
      }
    }

}

vector<sstate> model::Post(const sstate& ss)
{
    vector< vector<edge> > v_edges;
    vector<sstate> v_ss;
    vector<sstate> &sstates = v_ss;
    vector<string> synch_labels; 

    int a_index;
    vector<Combined_edge> edge_groups;
    for (auto loc_it = ss.loc_names.begin(); loc_it != ss.loc_names.end(); ++loc_it, ++a_index) {
        a_index = loc_it - ss.loc_names.begin();
	location &l = automata[a_index].get_location(*loc_it);
        vector<string> new_labels = automata[a_index].labels;
        combine(edge_groups, l, new_labels, loc_it==ss.loc_names.begin());
        //combine(l, new_labels, edge_groups, synch_labels, loc_it==ss.loc_names.begin());
    }

    for ( auto it = edge_groups.begin(); it != edge_groups.end(); it++) {
	sstate nss = ss;
	discrete_step(nss, it->edges);
	continuous_step(nss);
	sstates.push_back(nss);
    }

    return sstates;
}

sstate model::init_sstate()
{
    cout << "inside init_state()" << endl;
    sstate init;
    cout << "inside init_state()" << endl;
    string ln="";
    for ( auto it = automata.begin(); it != automata.end(); it++) {
	cout << "loc name " << it->init_loc_name << endl;
	init.loc_names.push_back(it->init_loc_name);
	ln += it->init_loc_name;
    }
    cout << "init name " << ln << endl; 
    init.cvx = C_Polyhedron(init_constraint->to_Linear_Constraint(cvars, dvars));
    cout << "cvx : " << init.cvx << endl;
    continuous_step(init);
    cout << "cvx after continuous step : " << init.cvx << endl;
    return init;
}

static bool contained_in(const sstate &ss, const list<sstate> &lss);
static void remove_included_sstates_in_a_list(const sstate &ss, list<sstate> &lss);

void model::SpaceExplorer()
{
    sstate init = init_sstate();
    list<sstate> next;
    list<sstate> current;
    current.push_back(init);
    int step = 0;
    while(true) {
	for ( auto it = current.begin(); it != current.end(); it++) {
	    vector<sstate> nsstates = Post(*it); 
	    for (auto iit = nsstates.begin(); iit != nsstates.end(); iit++) {
		if ( iit->cvx.is_empty()) continue;
		if ( is_bad(*iit)) {

		    throw ("A bad location is reached ... ");
                }
		if ( contained_in(*iit, current)) continue;
		if ( contained_in(*iit, next)) continue;
		if ( contained_in(*iit, Space)) continue;
		remove_included_sstates_in_a_list(*iit, next);
		next.push_back(*iit);
	    }
	}
	Space.splice(Space.end(), current);
	cout << "-----------------------------" << endl;
	cout << "Step : " << ++step << endl;
	cout << "Number of passed states : " << Space.size() + current.size()<< endl;
	cout << "Number of generated states : " << next.size() << endl;
	cout << "-----------------------------" << endl;
	if ( next.size() == 0)
	    return;
	current.splice(current.begin(), next);
    }
}

bool model::is_bad(const sstate &ss)
{
    for (auto it = ss.loc_names.begin(); it != ss.loc_names.end(); it++) {
	location &l = automata[it - ss.loc_names.begin()].get_location(*it);
	if (l.bad)
	    return true;
    }
    return false;
}

static bool contained_in(const sstate &ss, const list<sstate> &lss)
{
    for ( auto it = lss.begin(); it != lss.end(); it++)
	if ( it->contains(ss))
	    return true;
    return false;
}

static void remove_included_sstates_in_a_list(const sstate &ss, list<sstate> &lss)
{
    auto it = lss.begin();
    while (it != lss.end()){
	if (ss.contains(*it)) {
	    lss.erase(it++);
	    continue;
	}
	it++;
    }
}

void model::print()
{
    for (auto it = cvars.begin(); it != cvars.end(); it++)
    {
	if (it != cvars.begin())
	    cout << ",";
	cout << it->name;
    }
    cout << ": continous;" << endl;
    for (auto it = dvars.begin(); it != dvars.end(); it++)
    {
	if (it != dvars.begin())
	    cout << ",";
	cout << it->name;
    }
    cout << ": discrete;" << endl;
    for (auto it = automata.begin(); it != automata.end(); it++) {
	it->print();
    }
    cout << endl;
    cout << "init := " << endl;
    for ( auto it = automata.begin(); it != automata.end(); it++)
    {
	cout << "loc[" << it->name << "]==" << it->init_loc_name <<"& ";
	//cout << "loc[" << it->name << "]==" << it->init_loc->name <<"& ";
    }
    if ( init_constraint != nullptr)
	init_constraint->print();
    cout << ";" << endl;
    cout << "bad := ";
    bool first_bad = true;
    for ( auto it = automata.begin(); it != automata.end(); it++)
    {
	for ( auto jt = it->locations.begin(); jt != it->locations.end(); jt++) {
	    if ( jt->bad)  {
		if (first_bad) {
		    cout << "loc[" << it->name << "]==" << jt->name;
		    first_bad = false;
		}
		else {
		    cout << "& loc[" << it->name << "]==" << jt->name;
		}
	    }
	}
    }
    cout << ";" << endl;
}

void model::check_consistency()
{
    int i = 0;
    for ( auto it = automata.begin(); it != automata.end(); it++) {
	it->check_consistency(cvars, dvars);
	it->set_index(i++);
        sort(it->labels.begin(), it->labels.end());
    }
}

