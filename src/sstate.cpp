#include <sstate.hpp>
#include <iostream>
#include <ppl.hh>
#include "automaton.hpp"
#include "model.hpp"
#include "combined_edge.hpp"
#include "edge_factory.hpp"

using namespace std;
using namespace Parma_Polyhedra_Library::IO_Operators;

//std::map<Signature, std::vector<Combined_edge> > signature_to_combined_edges;
  
ostream & operator<<(ostream& os, const Signature &s)
{
    os << s.get_str();
    return os;
}

const std::string& Signature::get_str() const
{
    return str;
}

const unsigned& Signature::get_active_tasks() const
{
    return active_tasks;
}

Signature::Signature(const string &s) {
    str = s;
    active_tasks = 0;

    // To parse the list of active tasks
    vector<string> acts;
    //vector<int> acts_i;
    string act = "";
    for ( auto it = s.begin(); it != s.end(); it ++) {
        if ( *it <= '9' && *it >= '0') {
            //cout << "Got one active task " << *it << endl;
            act.push_back(*it);
        }
        else if ( act != "") {
            acts.push_back(act);
            act = "";
        }
    }
    if ( act != "")
        acts.push_back(act);

    for (auto &x : acts)
        active_tasks = active_tasks | 1 << atoi(x.c_str());
}

bool Signature::includes(const Signature& sig) const
{
    return active_tasks == (active_tasks | sig.active_tasks);
}

bool Signature::operator == (const Signature& sig) const
{
    return str == sig.str;
}

bool Signature::operator < (const Signature& sig) const
{
    return str < sig.str;
}

string Symbolic_State::get_loc_names() const
{
    string str = "";
    for (auto &x : locations)
        str += x->get_name();
    return str;
}

Signature Symbolic_State::get_signature() const
{
    return signature;
}

void Symbolic_State::update_signature() 
{
    signature = Signature(get_loc_names());
}

Symbolic_State::Symbolic_State(std::vector<Location *> &locs, 
			       const Valuations &dv) :
    locations(locs),
    dvars(dv)
{
    cvx = get_invariant_cvx();
    invariant_cvx = get_invariant_cvx();

    update_signature();
}

Symbolic_State::Symbolic_State(const std::vector<std::string> &loc_names, 
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
    update_signature();
}

//bool Symbolic_State::contains(const Symbolic_State &ss) const
//{
//    //for ( int i = 0; i < loc_names.size(); i++)
//    //if ( loc_names[i] != ss.loc_names[i])
//    //return false;
//    for (unsigned i = 0; i<locations.size(); i++) 
//	if (locations[i]->get_name() != ss.locations[i]->get_name()) 
//	    return false;
//    return cvx.contains(ss.cvx);
//}

bool Symbolic_State::contains(const shared_ptr<Symbolic_State> &pss) const
{
    if (not (signature == pss->signature)) return false;
    // for (unsigned i = 0; i<locations.size(); i++) 
    // 	if (locations[i]->get_name() != pss->locations[i]->get_name()) 
    // 	    return false;
    return cvx.contains(pss->cvx);
}

shared_ptr<Symbolic_State> Symbolic_State::clone() const
{
    return make_shared<Symbolic_State> (*this);
}


void Symbolic_State::print() const
{
  cout << "==============================" << endl;
  cout << "Signature : ";
      cout << signature.get_active_tasks() << endl;

  cout << "State name : ";
  for ( auto n : locations)
      cout << n->get_name();
  cout << endl;
  cout << "Discrete variables : \n";
  for (auto it = dvars.begin(); it != dvars.end(); it++) {
      if (it != dvars.begin())
          cout << ", ";
      cout << it->first << "==" << it->second;
  }
  cout << endl;
  cout << "CVX : " << endl;
  cout << cvx << endl;
  //cout << endl;
  cout << "------------------------------" << endl;
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
    /** 
     * The cvx obtained through updates in an
     * edge can have dimension higher than cvars.size().
     * */
    PPL::C_Polyhedron ass_cvx(cvars.size()*2);

    Variables_Set vs;

    for (auto &e : edges.get_edges()) {
	// find location source location with that name
	bool found = false;
	for (auto &p : locations) { 
	    if (p == &e.get_src_location()) {
		found = true;
		p = &e.get_dst_location();
	    }
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

    ass_cvx.add_constraints(cvx.constraints());
    //cvx.add_space_dimensions_and_embed(cvars.size());
    //ass_cvx.intersection_assign(cvx);
    //cvx.remove_higher_space_dimensions(cvars.size());
    /** 
     * Before intersecting ass_cvx and cvx, we must remove 
     * the lower cvars.size() dimensions.
     * */
    PPL::Variables_Set lower_dims;
    for ( unsigned i = 0; i < cvars.size(); i++)
      lower_dims.insert(PPL::Variable(i));
    ass_cvx.remove_space_dimensions(lower_dims);

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

const PPL::C_Polyhedron& Symbolic_State::get_cvx() const
{
    return cvx;
}

bool Symbolic_State::is_empty() const 
{
    return cvx.is_empty();
}

vector<shared_ptr<Symbolic_State> > Symbolic_State::post() const
{
    vector<shared_ptr<Symbolic_State> > sstates;
    vector<Combined_edge> eg = EDGE_FACTORY.get_edges(signature, locations);

    for (auto e : eg) {
	auto nss = clone(); 
	nss->discrete_step(e);
	nss->continuous_step();
	/** Do not forget to update the signature for the next sstate. */
	nss->update_signature();
	sstates.push_back(nss);
    }
    
    return sstates;
}

int Symbolic_State::total_memory_in_bytes() const
{
    return cvx.total_memory_in_bytes();
}

bool Symbolic_State::operator == (const Symbolic_State &ss) const
{
    if (locations.size() != ss.locations.size())
        return false;
    if (dvars.size() != ss.dvars.size())
        return false;
    for (int i = 0; i < locations.size(); i++)
        if (locations[i] != ss.locations[i])
            return false;
    for ( auto it = dvars.begin(), jt = ss.dvars.begin(); it != dvars.end(); ++it, ++jt)
        if (it->first != jt->first || it->second != jt->second)
            return false;
        
    return cvx.contains(ss.cvx) && ss.cvx.contains(cvx) 
            && invariant_cvx.contains(ss.invariant_cvx) && ss.invariant_cvx.contains(invariant_cvx);

}

bool Symbolic_State::equals(const std::shared_ptr<Symbolic_State> &pss) const
{
    if (locations.size() != pss->locations.size())
        return false;
    if (dvars.size() != pss->dvars.size())
        return false;
    for (int i = 0; i < locations.size(); i++)
        if (locations[i] != pss->locations[i])
            return false;
    for ( auto it = dvars.begin(), jt = pss->dvars.cbegin(); it != dvars.end(); ++it, ++jt)
        if (it->first != jt->first || it->second != jt->second)
            return false;
    //const Valuations &dvs = dvars; 
    //for ( auto it = dvs.begin(), jt = pss->dvars.cbegin(); it != dvs.end(); ++it, ++jt)
    //    if (it->first != jt->first || it->second != jt->second)
    //        return false;

    if ( ! (signature == pss->signature))
        return false;
    return cvx.contains(pss->cvx) && pss->cvx.contains(cvx) 
            && invariant_cvx.contains(pss->invariant_cvx) && pss->invariant_cvx.contains(invariant_cvx);

}
