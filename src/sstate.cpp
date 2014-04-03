#include <sstate.hpp>
#include <iostream>
#include <ppl.hh>
#include "automaton.hpp"
#include "model.hpp"
#include "combined_edge.hpp"

using namespace std;
using namespace Parma_Polyhedra_Library::IO_Operators;

Symbolic_State::Symbolic_State(std::vector<Location *> &locs, 
			       const Valuations &dv) :
    locations(locs),
    dvars(dv)
{
    cvx = get_invariant_cvx();
    invariant_cvx = get_invariant_cvx();
}

Symbolic_State::Symbolic_State(std::vector<std::string> &loc_names, 
			       const Valuations &dv,
			       const PPL::C_Polyhedron &pol) :
    dvars(dv),
    cvx(pol)
{
    invariant_cvx = get_invariant_cvx();   
    int i = 0;
    for (auto l : loc_names) {  
	Location *p = &(MODEL.get_automaton_at(i).get_location_by_name(l));
	locations.push_back(p);
	i++;
    }
}

bool Symbolic_State::contains(const Symbolic_State &ss) const
{
    //for ( int i = 0; i < loc_names.size(); i++)
    //if ( loc_names[i] != ss.loc_names[i])
    //return false;
    for (unsigned i = 0; i<locations.size(); i++) 
	if (locations[i]->get_name() != ss.locations[i]->get_name()) 
	    return false;
    return cvx.contains(ss.cvx);
}

void Symbolic_State::print() const
{
  cout << "-----------------------------" << endl;
  cout << "State name : ";
  for ( auto n : locations)
      cout << n->get_name();
  cout << endl;
  cout << "CVX : " << endl;
  cout << cvx << endl;
  cout << endl;
  cout << "-----------------------------" << endl;
}


bool Symbolic_State::is_bad() const 
{
    for (auto p : locations) 
	if (p->is_bad()) return true;

    return false;

}

void Symbolic_State::continuous_step()
{
    VariableList cvars = MODEL.get_cvars();
    PPL::C_Polyhedron r_cvx(cvars.size());
    PPL::C_Polyhedron i_cvx(cvars.size());
    
    VariableList lvars = cvars;
    for (auto p: locations) {
	Linear_Constraint lc;
	i_cvx.add_constraints(p->invariant_to_Linear_Constraint(cvars, dvars));
	r_cvx.add_constraints(p->rates_to_Linear_Constraint(cvars, dvars, lvars));
    }

    for (auto &v : lvars) {
	PPL::Variable var = get_ppl_variable(cvars, v);
	Linear_Expr le;
	le += 1;
	AT_Constraint atc = (var == le);
	r_cvx.add_constraint(atc);
    }
    cvx.time_elapse_assign(r_cvx);
    cvx.intersection_assign(i_cvx);
}


void Symbolic_State::discrete_step(Combined_edge &edges)
{
    VariableList cvars = MODEL.get_cvars();

    PPL::C_Polyhedron guard_cvx(cvars.size());
    PPL::C_Polyhedron ass_cvx(cvars.size());

    Variables_Set vs;

    for (auto &e : edges.get_edges()) {
	// find location source location with that name
	bool found = false;
	for (auto &p : locations) 
	    if (p == &e.get_src_location()) {
		found = true;
		p = &e.get_dst_location();
	    }
	if (!found) throw string("ERROR!!! Cannot find src location ") +
			e.get_src_location().get_name();
	// ss.loc_names[e.get_automaton_index()] = e.get_dest();
	guard_cvx.add_constraints(e.guard_to_Linear_Constraint(cvars, dvars));
	Variables_Set vs2 = e.get_assignment_vars(cvars);
	vs.insert(vs2.begin(), vs2.end());
	// for (auto &a : e.assignments)
	//     vs.insert(get_variable(a.get_var(), cvars));
	ass_cvx.add_constraints(e.ass_to_Linear_Constraint(cvars, dvars));
    }
    cvx.intersection_assign(guard_cvx);
    cvx.unconstrain(vs);
    cvx.intersection_assign(ass_cvx);
    cvx.intersection_assign(invariant_cvx);
}


PPL::C_Polyhedron Symbolic_State::get_invariant_cvx()
{
    VariableList cvars = MODEL.get_cvars();
    PPL::C_Polyhedron i_cvx(cvars.size());
    //for ( auto it = automata.begin(); it != automata.end(); it++){
    for (auto p : locations) {
	Linear_Constraint lc;
	// string ln = ss.loc_names[it-automata.begin()];
	// Location &l = it->get_location_by_name(ln);

	i_cvx.add_constraints(p->invariant_to_Linear_Constraint(cvars, dvars));
    }
    return i_cvx;
}

bool Symbolic_State::is_empty() const 
{
    return cvx.is_empty();
}

void combine(vector<Combined_edge> &edge_groups, const Location &l, 
	     const vector<string> new_labels,
	     bool first) 
{
    if (first) {
	vector<Edge> outgoings = l.get_edges();
	for (auto iit = outgoings.begin(); iit != outgoings.end(); iit++) {
            Combined_edge egroup(*iit, iit->get_label(), new_labels);
            // egroup.edges.push_back(*iit);
            // //if ( iit->sync_label != "") {
            //   egroup.sync_label = (iit->sync_label);
            //   egroup.sync_set = new_labels;
            // //}
            edge_groups.push_back(egroup);
	}
	return;
    } 
    vector<Combined_edge> copy = edge_groups;
    edge_groups.clear();
    // to combine every outgoing from "l" with every "edge group"
    vector<Edge> outgoings = l.get_edges();
    for ( auto &egroup : copy) {
      for ( auto it = outgoings.begin(); it != outgoings.end(); it++) {
        vector<Combined_edge> com = egroup.combine(*it, new_labels);
        for ( auto &eg : com)
          if ( not contains(edge_groups, eg))
            edge_groups.push_back(eg);
      }
    }
}


vector<Symbolic_State> Symbolic_State::post() const
{
    vector< vector<Edge> > v_edges;
    vector<Symbolic_State> v_ss;
    vector<Symbolic_State> &sstates = v_ss;
    vector<string> synch_labels; 

    //int a_index;
    vector<Combined_edge> edge_groups;
    //for (auto loc_it = ss.loc_names.begin(); loc_it != ss.loc_names.end(); ++loc_it, ++a_index) {
    bool first = true;
    for (auto p : locations) {
        //a_index = loc_it - ss.loc_names.begin();
	//Location &l = automata[a_index].get_location_by_name(*loc_it);
        vector<string> new_labels = p->get_automaton().get_labels(); 
	    //automata[a_index].get_labels();
        combine(edge_groups, *p, new_labels, first);
        //combine(l, new_labels, edge_groups, synch_labels, loc_it==ss.loc_names.begin());
	first = false;
    }
    for (auto e : edge_groups) {
	Symbolic_State nss = *this;
	nss.discrete_step(e);
	nss.continuous_step();
	sstates.push_back(nss);
    }
    
    return sstates;
}

int Symbolic_State::total_memory_in_bytes() const
{
    return cvx.total_memory_in_bytes();
}
