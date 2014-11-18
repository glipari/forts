#include <sstate.hpp>
#include <iostream>
#include <ppl.hh>
#include "automaton.hpp"
#include "model.hpp"
#include "combined_edge.hpp"
#include "edge_factory.hpp"

using namespace std;
using namespace Parma_Polyhedra_Library::IO_Operators;

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

Symbolic_State::Symbolic_State() {}

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
			       const PPL::NNC_Polyhedron &pol) :
    dvars(dv),
    cvx(pol)
{
    int i = 0;
    for (auto l : loc_names) {  
	Location *p = &(MODEL.get_automaton_at(i).get_location_by_name(l));
	locations.push_back(p);
	i++;
    }
    invariant_cvx = get_invariant_cvx();   
    update_signature();
}

bool Symbolic_State::contains(const shared_ptr<Symbolic_State> &pss) const
{
    //if (not (signature == pss->signature)) return false;
    if ( not (get_loc_names() == pss->get_loc_names()))
      return false;
    auto it = dvars.begin();
    auto jt = pss->get_dvars().begin();
    while ( it != dvars.end()) {
      if( it->second != jt->second)
        return false;
      it ++;
      jt ++;
    }
    return cvx.contains(pss->cvx);
}

shared_ptr<Symbolic_State> Symbolic_State::clone() const
{
    return make_shared<Symbolic_State> (*this);
}


void Symbolic_State::print() const
{
  cout << "==============================" << endl;
  //cout << "Signature : ";
  //    cout << signature.get_active_tasks() << endl;

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
  //cout << "beginning of a continuous step. " << endl;
  VariableList cvars = MODEL.get_cvars();
  PPL::NNC_Polyhedron r_cvx(cvars.size());
  
  VariableList lvars = cvars;
  
  for (auto p: locations) {
    Linear_Constraint lc;
    //r_cvx.add_constraints(p->rates_to_Linear_Constraint(cvars, dvars));
    r_cvx.add_constraints(p->flow_to_Linear_Constraint(cvars, dvars));
    auto flow = p->get_flow();
    auto it = lvars.begin();
    while ( it != lvars.end()) {
      if(flow.has_variable(*it)) {
        it = lvars.erase(it);
      }
      else it ++;
    }
  }

  for (auto &v : lvars) {
    PPL::Variable var = get_ppl_variable(cvars, v);
    Linear_Expr le;
    le += 1;
    AT_Constraint atc = (var == le);
    r_cvx.add_constraint(atc);
  }
    
  cvx.time_elapse_assign(r_cvx);
  cvx.intersection_assign(invariant_cvx);
  invariant_cvx.remove_higher_space_dimensions(0);
  //print();
  //cout << "end of a continuous step. " << endl;
}


/** The discrete step for a symbolic state. */
void Symbolic_State::discrete_step(const Combined_edge &edges)
{
  //cout << "beginning of a discrete step. " << endl;
  //print();
  /** Always keep a copy of continuous variables. */
  VariableList cvars = MODEL.get_cvars();

  /** The guard on an edge. */
  PPL::NNC_Polyhedron guard_cvx(cvars.size());

  /**
   * The assignment on an edge.
   * The cvx obtained through updates in an
   * edge can have dimension higher than cvars.size().
   **/
  //PPL::NNC_Polyhedron ass_cvx(cvars.size()*2);

  /**
   * According to the new syntax of assignment in an edge,
   * there is no need to increase the dimensions of 
   * the polyhedron for assignments.
   **/
  PPL::NNC_Polyhedron ass_cvx(cvars.size());
  
  /** A set of PPL variables that are involved with the assignment statement in an edge. */
  //Variables_Set vs;
    
  /** The "locations" is a vector of Location pointers. */
  auto l = locations;

  VariableList ass_cvars;
  for (auto &e : edges.get_edges()) {
    /** Find location source location with that name */
    bool found = false;
    for (auto &p : locations) { 
      if (p == &e.get_src_location()) {
        found = true;
        p = &e.get_dst_location();
        break;
      }
    }
    
    if (!found) 
      throw string("ERROR!!! Cannot find src location ") +
        e.get_src_location().get_name();
    // ss.loc_names[e.get_automaton_index()] = e.get_dest();
	
    guard_cvx.add_constraints(e.guard_to_Linear_Constraint(cvars, dvars));
	
    //Variables_Set vs2 = e.get_assignment_vars(cvars);
    //vs.insert(vs2.begin(), vs2.end());
    // for (auto &a : e.assignments)
    //     vs.insert(get_variable(a.get_var(), cvars));
    ass_cvx.add_constraints(e.ass_to_Linear_Constraint(cvars, dvars));
    auto assignments = e.get_assignments();
    for ( auto & c : cvars) {
      if ( contains_in_a_set<std::string>(ass_cvars, c))
        continue;
      for ( auto & x : assignments) {
        if ( x.has_variable(c)) {
          ass_cvars.insert(ass_cvars.end(), c);
          break;
        }
      }
    }
  }
  
  cvx.intersection_assign(guard_cvx);
  
  //ass_cvx.add_constraints(cvx.constraints());
    //cvx.add_space_dimensions_and_embed(cvars.size());
    //ass_cvx.intersection_assign(cvx);
    //cvx.remove_higher_space_dimensions(cvars.size());
  /** 
    * Before intersecting ass_cvx and cvx, we must remove 
    * the lower cvars.size() dimensions.
  * */
  //PPL::Variables_Set lower_dims;
  //for ( unsigned i = 0; i < cvars.size(); i++)
    //lower_dims.insert(PPL::Variable(i));
  //ass_cvx.remove_space_dimensions(lower_dims);
  
  //cvx.unconstrain(vs);
  Variables_Set ass_vs;
  for ( auto & x : ass_cvars)
    ass_vs.insert(get_ppl_variable(cvars, x));
  cvx.unconstrain(ass_vs);
  cvx.intersection_assign(ass_cvx);
    // To build the invariant_cvx, which is latter used and destroyed in continuous_step()
  invariant_cvx = get_invariant_cvx();
  cvx.intersection_assign(invariant_cvx);
  
  incoming_edge = edges;
  incoming_edge.set_locations(l);
    //cout << "leave a discrete step ... " << endl;
  //print();
  //cout << "end of a discrete step. " << endl;
}


PPL::NNC_Polyhedron Symbolic_State::get_invariant_cvx()
{
    VariableList cvars = MODEL.get_cvars();
    PPL::NNC_Polyhedron i_cvx(cvars.size());
    //for ( auto it = automata.begin(); it != automata.end(); it++){
    for (auto p : locations) {
	Linear_Constraint lc;
	// string ln = ss.loc_names[it-automata.begin()];
	// Location &l = it->get_location_by_name(ln);

	i_cvx.add_constraints(p->invariant_to_Linear_Constraint(cvars, dvars));
    }
    return i_cvx;
}

const PPL::NNC_Polyhedron& Symbolic_State::get_cvx() const
{
    return cvx;
}

bool Symbolic_State::is_empty() const 
{
    return cvx.is_empty();
}

//void combine(vector<Combined_edge> &edge_groups, const Location &l, 
//	     const vector<string> new_labels,
//	     bool first) 
//{
//    if (first) {
//	    vector<Edge> outgoings = l.get_edges();
//        if (outgoings.size() == 0) {
//            Combined_edge egroup(new_labels);
//            edge_groups.push_back(egroup);
//	        return;
//        }
//	    for (auto iit = outgoings.begin(); iit != outgoings.end(); iit++) {
//            Combined_edge egroup(*iit, iit->get_label(), new_labels);
//            // egroup.edges.push_back(*iit);
//            // //if ( iit->sync_label != "") {
//            //   egroup.sync_label = (iit->sync_label);
//            //   egroup.sync_set = new_labels;
//            // //}
//            edge_groups.push_back(egroup);
//	    }
//	    return;
//    } 
//    vector<Combined_edge> copy = edge_groups;
//    edge_groups.clear();
//    // to combine every outgoing from "l" with every "edge group"
//    vector<Edge> outgoings = l.get_edges();
//    for ( auto &egroup : copy) {
//      if ( outgoings.size() == 0) {
//        vector<Combined_edge> com = egroup.combine(new_labels);
//        for ( auto &eg : com)
//          if ( not contains(edge_groups, eg))
//            edge_groups.push_back(eg);
//      }
//      for ( auto it = outgoings.begin(); it != outgoings.end(); it++) {
//        vector<Combined_edge> com = egroup.combine(*it, new_labels);
//        for ( auto &eg : com)
//          if ( not contains(edge_groups, eg))
//            edge_groups.push_back(eg);
//      }
//    }
//}
//void combine(vector<Combined_edge> &edge_groups, const Location &l, 
//	     const vector<string> new_labels,
//	     bool first) 
//{
//    if (first) {
//	vector<Edge> outgoings = l.get_edges();
//	for (auto iit = outgoings.begin(); iit != outgoings.end(); iit++) {
//            Combined_edge egroup(*iit, iit->get_label(), new_labels);
//            // egroup.edges.push_back(*iit);
//            // //if ( iit->sync_label != "") {
//            //   egroup.sync_label = (iit->sync_label);
//            //   egroup.sync_set = new_labels;
//            // //}
//            edge_groups.push_back(egroup);
//	}
//	return;
//    } 
//    vector<Combined_edge> copy = edge_groups;
//    edge_groups.clear();
//    // to combine every outgoing from "l" with every "edge group"
//    vector<Edge> outgoings = l.get_edges();
//    for ( auto &egroup : copy) {
//      for ( auto it = outgoings.begin(); it != outgoings.end(); it++) {
//        vector<Combined_edge> com = egroup.combine(*it, new_labels);
//        for ( auto &eg : com)
//          if ( not contains(edge_groups, eg))
//            edge_groups.push_back(eg);
//      }
//    }
//}


vector<shared_ptr<Symbolic_State> > Symbolic_State::post() const
{
    vector<shared_ptr<Symbolic_State> > sstates;
    //auto it = signature_to_combined_edges.find(signature);
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

void Symbolic_State::mark_prior(shared_ptr<Symbolic_State> p)
{
    prior = p;
}

shared_ptr<Symbolic_State> Symbolic_State::get_prior() const
{
    return prior;
}

Combined_edge Symbolic_State::get_incoming_edge() const
{
    return incoming_edge;
}

const vector<Location *> & Symbolic_State::get_locations() const
{
    return locations;
}
    
bool Symbolic_State::no_outgoings() const
{
    for (auto &x : locations) {
        if (x->get_edges().size() != 0)
            return false;
    }
    return true;
}

void Symbolic_State::refine_cvx(const PPL::NNC_Polyhedron &poly)
{
    cvx.intersection_assign(poly);
}

bool Symbolic_State::is_valid() const
{
    return valid;
}

void Symbolic_State::invalidate()
{
    valid = false;
}
