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

template<class C, class X>
bool contains(const C &c, const X &x) 
{
    return std::find(c.begin(), c.end(), x) != c.end();
}

void combine(const location &l, 
	     const vector<string> new_labels,
	     vector< vector<edge> > &v_edges, 
	     vector<string> &synch_labels,
	     bool first) 
{
    if (first) {
	for (auto iit = l.outgoings.begin(); iit != l.outgoings.end(); iit++) {
	    vector<edge> v;
	    v.push_back(*iit);
	    v_edges.push_back(v);
	}
	synch_labels = new_labels;
	return;
    } else {
	// this is the intersection of the two sets of synch labels
	vector<string> common_labels;
	std::set_intersection(synch_labels.begin(), 
			      synch_labels.end(), 
			      new_labels.begin(),
			      new_labels.end(),
			      std::back_inserter(common_labels));

        vector< vector<edge> > origin_v_edges = v_edges;
        //v_edges.clear();
        vector< vector<edge> > sub_v_edges;
	for (auto &e : l.outgoings) { 
	    // edge is the current outgoing edge in location l
	    if (e.sync_label == "") {// or 
		//not contains(common_labels, e.sync_label)) {
		// no synchronisation, 
		// the number of elements in v_edge is duplicated: 
		// we need to add one element to each vector: it can be 
		// a stuttering or one containing the edge
		// 
		// plus we add a new vector that contains all 
		// stuttering except the edge element
                for ( auto &edge_group : origin_v_edges) {
                  // duplicate already existing edge groups
                  //vector<edge> duplicated_edge_group = edge_group;
                  //sub_v_edges.push_back(duplicated_edge_group);
                  // append e to already existing edge groups
                  vector<edge> eg = edge_group;
                  eg.push_back(e);
                  sub_v_edges.push_back(eg);
                  // a new edge group with only "e" inside
                  vector<edge> new_edge_group = {e};
                  sub_v_edges.push_back(new_edge_group);
                }
	    } else if ( not contains(common_labels, e.sync_label)) {
              // there is synchronisation,
              // and it does not synchronize with any previously visited edge
              vector<edge> new_edge_group = {e};
              sub_v_edges.push_back(new_edge_group);
              // "e" can be appended to an edge group such that all edges inside it have empty synch label ""
              for (auto &edge_group : origin_v_edges) {
                bool empty_sync = true;
                for (auto &e1 : edge_group)
                  if (e1.sync_label != "") {
                    empty_sync = false;
                    break;
                  }
                if (empty_sync) {
                  vector<edge> eg = edge_group;
                  eg.push_back(e);
                  sub_v_edges.push_back(eg);
                }
              }
            }
            else {
		for (auto &edge_group : origin_v_edges) {
		    for (auto existing_edge : edge_group) {
			if (e.sync_label == existing_edge.sync_label) {
			    // yes they synchronise
			    // Add this only to the corresponding edge_group
			    // and skip the rest
                            //edge_group.push_back(e);
                            vector<edge> eg = edge_group;
                            eg.push_back(e);
                            sub_v_edges.push_back(eg);
                            // remove the half synchronized edge group in v_edges
                            auto xit = v_edges.begin();
                            while (xit != v_edges.end()) {
                              bool existing = false;
                              for ( auto &xxit : *xit) {
                                if (xxit.sync_label == existing_edge.sync_label)
                                  existing = true;
                                if (existing) break;
                              }
                              if (existing) v_edges.erase(xit);
                              else xit ++;
                            }
                            break;
			}
			// otherwise, do nothing
		    }
		}
	    }
	}
        // store all new combinations into v_edges
        for (auto &xx : sub_v_edges)
          v_edges.push_back(xx);
    }
    vector<string> result;
    std::set_union(synch_labels.begin(), 
		   synch_labels.end(), 
		   new_labels.begin(),
		   new_labels.end(),
		   std::back_inserter(result));
    synch_labels = result;
}


vector<sstate> model::Post(const sstate& ss)
{
    vector< vector<edge> > v_edges;
    vector<sstate> v_ss;
    vector<sstate> &sstates = v_ss;
    vector<string> synch_labels; 

    int a_index;
    vector< vector<edge> > edge_groups;
    for (auto loc_it = ss.loc_names.begin(); loc_it != ss.loc_names.end(); ++loc_it, ++a_index) {
        a_index = loc_it - ss.loc_names.begin();
	location &l = automata[a_index].get_location(*loc_it);
        vector<string> new_labels = automata[a_index].labels;
        combine(l, new_labels, edge_groups, synch_labels, loc_it==ss.loc_names.begin());
    }
        cout << "-----------------------------"<< endl;
        cout << "Start from : "; 
        for ( auto &y : ss.loc_names)
          cout << y ;
        cout << endl;
        for ( auto it = edge_groups.begin(); it != edge_groups.end(); it++){
          for ( auto iit = it->begin(); iit != it->end(); iit++)
            iit->print();
          cout << endl;
        }
          
        cout << "-----------------------------"<< endl;

    for ( auto it = edge_groups.begin(); it != edge_groups.end(); it++) {
	sstate nss = ss;
	discrete_step(nss, *it);
        //cout <<"after a discrete step : " << endl;
        //for ( auto &l : nss.loc_names)
        //  cout<<l;
        //cout << endl;
        //cout << nss.cvx << endl;
	continuous_step(nss);
        //cout <<"after a continuous step : " << endl;
        //for ( auto &l : nss.loc_names)
        //  cout<<l;
        //cout << endl;
        //cout << nss.cvx << endl;
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
		if ( is_bad(*iit))
		    throw ("A bad location is reached ... ");
        for ( auto &l : iit->loc_names)
          cout<<l;
        cout << endl;
        cout << iit->cvx << endl;
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

//void combine(const location &l, 
//	     const vector<string> new_labels,
//	     vector< vector<edge> > &v_edges, 
//	     vector<string> &synch_labels,
//	     bool first) 
//{
//    if (first) {
//	for (auto iit = l.outgoings.begin(); iit != l.outgoings.end(); iit++) {
//	    vector<edge> v;
//	    v.push_back(*iit);
//	    v_edges.push_back(v);
//	}
//	synch_labels = new_labels;
//	return;
//    } else {
//	// this is the intersection of the two sets of synch labels
//	vector<string> common_labels;
//	std::set_intersection(synch_labels.begin(), 
//			      synch_labels.end(), 
//			      new_labels.begin(),
//			      new_labels.end(),
//			      std::back_inserter(common_labels));
//
//        vector< vector<edge> > origin_v_edges = v_edges;
//        v_edges.clear();
//	for (auto &e : l.outgoings) { 
//            vector< vector<edge> > sub_v_edges;
//	    // edge is the current outgoing edge in location l
//	    if (e.sync_label == "") {// or 
//		//not contains(common_labels, e.sync_label)) {
//		// no synchronisation, 
//		// the number of elements in v_edge is duplicated: 
//		// we need to add one element to each vector: it can be 
//		// a stuttering or one containing the edge
//		// 
//		// plus we add a new vector that contains all 
//		// stuttering except the edge element
//                for ( auto &edge_group : origin_v_edges) {
//                  // duplicate already existing edge groups
//                  vector<edge> duplicated_edge_group = edge_group;
//                  sub_v_edges.push_back(duplicated_edge_group);
//                  // append e to already existing edge groups
//                  vector<edge> eg = edge_group;
//                  eg.push_back(e);
//                  sub_v_edges.push_back(eg);
//                  // a new edge group with only "e" inside
//                  vector<edge> new_edge_group = {e};
//                  sub_v_edges.push_back(new_edge_group);
//                }
//	    } else if ( not contains(common_labels, e.sync_label)) {
//              // there is synchronisation,
//              // and it does not synchronize with any previously visited edge
//              vector<edge> new_edge_group = {e};
//              sub_v_edges.push_back(new_edge_group);
//              // "e" can be appended to an edge group such that all edges inside it have empty synch label ""
//              for (auto &edge_group : origin_v_edges) {
//                bool empty_sync = true;
//                for (auto &e1 : edge_group)
//                  if (e1.sync_label != "") {
//                    empty_sync = false;
//                    break;
//                  }
//                if (empty_sync) {
//                  vector<edge> eg = edge_group;
//                  eg.push_back(e);
//                  sub_v_edges.push_back(eg);
//                }
//              }
//            }
//            else {
//		for (auto &edge_group : origin_v_edges) {
//		    for (auto existing_edge : edge_group) {
//			if (e.sync_label == existing_edge.sync_label) {
//			    // yes they synchronise
//			    // Add this only to the corresponding edge_group
//			    // and skip the rest
//                            //edge_group.push_back(e);
//                            vector<edge> eg = edge_group;
//                            eg.push_back(e);
//                            sub_v_edges.push_back(eg);
//                            break;
//			}
//			// otherwise, do nothing
//		    }
//		}
//	    }
//            for (auto &xx : sub_v_edges)
//              v_edges.push_back(xx);
//	}
//    }
//    vector<string> result;
//    std::set_union(synch_labels.begin(), 
//		   synch_labels.end(), 
//		   new_labels.begin(),
//		   new_labels.end(),
//		   std::back_inserter(result));
//    synch_labels = result;
//}
