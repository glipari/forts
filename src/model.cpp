#include <iostream>
#include <algorithm>
#include <fstream>
#include <time.h>

#include <expression.hpp>
#include <model.hpp>

#include "combined_edge.hpp"
#include "widened_sstate.hpp"
#include "widened_sstate_ex.hpp"
#include "widened_merge_sstate.hpp"
#include "box_widened_sstate.hpp"
#include "dbm_sstate.hpp"
#include "oct_sstate.hpp"
#include "edge_factory.hpp"

using namespace std;
namespace PPL=Parma_Polyhedra_Library;
using namespace Parma_Polyhedra_Library::IO_Operators;

Model *Model::the_instance = nullptr;

Model::Model()
{
  merge = false;
}

// PPL::NNC_Polyhedron Model::get_invariant_cvx(Symbolic_State &ss)
// {
//     PPL::NNC_Polyhedron invariant_cvx(cvars.size());
//     for ( auto it = automata.begin(); it != automata.end(); it++){
// 	Linear_Constraint lc;
// 	string ln = ss.loc_names[it-automata.begin()];
// 	Location &l = it->get_location_by_name(ln);

// 	invariant_cvx.add_constraints(l.invariant_to_Linear_Constraint(cvars, dvars));
//     }
//     return invariant_cvx;
// }


Model &Model::get_instance()
{
//    static Model m;
    if (the_instance == nullptr) the_instance = new Model();
    return *the_instance;
}

void Model::reset() 
{
    delete the_instance;
    the_instance = new Model();
    //cache_reset();
    EdgeFactory::reset();
}

// void Model::continuous_step(Symbolic_State &ss)
// {
//     // 1) To do time_elapse_assign
//     PPL::NNC_Polyhedron rates_cvx(cvars.size());
//     PPL::NNC_Polyhedron invariant_cvx(cvars.size());
//     VariableList lvars = cvars;
//     for ( auto it = automata.begin(); it != automata.end(); it++){
// 	Linear_Constraint lc;
// 	string ln = ss.loc_names[it-automata.begin()];
// 	Location &l = it->get_location_by_name(ln);

// 	invariant_cvx.add_constraints(l.invariant_to_Linear_Constraint(cvars, dvars));
// 	rates_cvx.add_constraints(l.rates_to_Linear_Constraint(cvars, dvars, lvars));
//         //cout << "rates : " << rates_cvx << endl;
//         //cout << "invariant : " << invariant_cvx << endl;
    
//     }

//     // If a variable's rate is not specified in the location, it's assumed to be 1
//     for ( auto it = lvars.begin(); it != lvars.end(); it++) {
// 	PPL::Variable v = get_ppl_variable(cvars, *it);
// 	Linear_Expr le;
// 	le += 1;
// 	AT_Constraint atc = (v==le);
// 	rates_cvx.add_constraint(atc);
//     }
//     ss.cvx.time_elapse_assign(rates_cvx);
//     // 2) intersect with invariant
//     ss.cvx.intersection_assign(invariant_cvx);
// }

// TODO : remember to change the e parameter into a const reference
//void Model::discrete_step(Symbolic_State &ss, Combined_edge &edges)
//{
//    // PPL::NNC_Polyhedron guard_cvx(cvars.size());
//    // PPL::NNC_Polyhedron ass_cvx(cvars.size());
//    // Variables_Set vs;
//
//    // for (auto &e : edges.get_edges()) {
//    // 	ss.loc_names[e.get_automaton_index()] = e.get_dest();
//    // 	guard_cvx.add_constraints(e.guard_to_Linear_Constraint(cvars, dvars));
//    // 	Variables_Set vs2 = e.get_assignment_vars(cvars);
//    // 	vs.insert(vs2.begin(), vs2.end());
//    // 	// for (auto &a : e.assignments)
//    // 	//     vs.insert(get_variable(a.get_var(), cvars));
//    // 	ass_cvx.add_constraints(e.ass_to_Linear_Constraint(cvars, dvars));
//    // }
//    // ss.cvx.intersection_assign(guard_cvx);
//    // ss.cvx.unconstrain(vs);
//    // ss.cvx.intersection_assign(ass_cvx);
//    // ss.cvx.intersection_assign(get_invariant_cvx(ss));
//    ss.discrete_step(edges);
//}

void Model::discrete_step(shared_ptr<Symbolic_State> &pss, Combined_edge &edges)
{
    pss->discrete_step(edges);
}



vector<shared_ptr<Symbolic_State> > Model::Post(const shared_ptr<Symbolic_State>& pss)
{

    return pss->post();
}

vector<shared_ptr<Symbolic_State> > Model::discrete_steps(const shared_ptr<Symbolic_State>& pss)
{

    return pss->discrete_steps();
}


shared_ptr<Symbolic_State> Model::init_sstate()
{
    vector<Location *> locs;
    vector<std::string> loc_names;
    PPL::NNC_Polyhedron cvx(cvars.size());

    string ln="";
    for (auto it = automata.begin(); it != automata.end(); it++) {
	//cout << "loc name " << it->get_init_location() << endl;
	//init.loc_names.push_back(it->get_init_location());
	loc_names.push_back(it->get_init_location());
	locs.push_back(&(it->get_location_by_name(it->get_init_location())));
	ln += it->get_init_location();
    }
    //cout << "init name " << ln << endl; 
    cvx = NNC_Polyhedron(init_constraint.to_Linear_Constraint(cvars, dvars));
    //Symbolic_State init(loc_names, dvars, cvx);

    //auto init = make_shared<Symbolic_State>(loc_names, dvars, cvx);

    shared_ptr<Symbolic_State> init;

    if (sstate_type == WIDENED)
        init = make_shared<Widened_Symbolic_State>(loc_names, dvars, cvx);
    if (sstate_type == WIDENED_EX)
        init = make_shared<Widened_Symbolic_State_ex>(loc_names, dvars, cvx);
    else if (sstate_type == WIDENED_MERGE) {
      merge = true;
      init = make_shared<Widened_Merge_Symbolic_State>(loc_names, dvars, cvx);
    }
    else if (sstate_type == BOX_WIDENED)
        init = make_shared<Box_Widened_Symbolic_State>(loc_names, dvars, cvx);
    //else if (sstate_type == DBM)
    //    init = make_shared<DBM_Symbolic_State>(loc_names, dvars, cvx);
    //else if (sstate_type == OCT) {
    //    init = make_shared<OCT_Symbolic_State>(loc_names, dvars, cvx);
    //}
    else
        init = make_shared<Symbolic_State>(loc_names, dvars, cvx);

    //init->print();
    init->continuous_step();
    init->do_something();
    //cout << "cvx after continuous step : ";
    //init->print();
    cout << "cvx after continuous step : ";
    init->print();
    return init;
}

// static bool contained_in(const shared_ptr<Symbolic_State> &ss, const list<shared_ptr<Symbolic_State> > &lss);
// static int remove_included_sstates_in_a_list(const shared_ptr<Symbolic_State> &ss, list<shared_ptr<Symbolic_State> > &lss);

void Model::SpaceExplorer()
{
    clock_t begin, end;
    clock_t time_spent;
    begin = clock();
    shared_ptr<Symbolic_State> init = init_sstate();
    list<shared_ptr<Symbolic_State> > next;
    list<shared_ptr<Symbolic_State> > current;
    current.push_back(init);
    int step = 0; 

    stats = model_stats{};

    while(true) {
	for ( auto it = current.begin(); it != current.end(); it++) {
          
          post_stat.start();
          vector<shared_ptr<Symbolic_State> > nsstates;
          nsstates = Post(*it); 
	  post_stat.stop();
	  stats.total_states += nsstates.size();

          for (auto iit = nsstates.begin(); iit != nsstates.end(); iit++) {
            
            //(*iit)->mark_prior(*it);
            if ( (*iit)->is_empty()) {
              stats.eliminated++;
              continue;
            }
            if ( (*iit)->is_bad()) {
              throw ("A bad location is reached ... ");
            }
            if ( contained_in(*iit, current) ) {
              stats.eliminated++;
              continue;
            }
            if ( contained_in(*iit, next)) {
              stats.eliminated++;
              continue;
            }
            if ( contained_in(*iit, Space)) {
              stats.eliminated++;
              continue;
            }
            if (merge)
              if ( merged_in(*iit, next)) {
                stats.eliminated++;
                continue;
              }
            (*iit)->do_something();
            stats.past_elim_from_next += remove_included_sstates_in_a_list(*iit, next);
            //stats.past_elim_from_current += remove_included_sstates_in_a_list(*iit, current);
            ////stats.past_elim_from_space += remove_included_sstates_in_a_list(*iit, Space);
            next.push_back(*iit);
          }
        }

        for ( auto & xit : current) xit->clear();

	Space.splice(Space.end(), current);
	cout << "-----------------------------" << endl;
	cout << "Step : " << ++step << endl;
	cout << "Number of passed states : " << Space.size() + current.size()<< endl;
	cout << "Number of generated states : " << next.size() << endl;
	cout << "-----------------------------" << endl;
	if ( next.size() == 0)
	    break;
	current.splice(current.begin(), next);
	//if ( not merge) current.splice(current.begin(), next);
        //else {
        //  for ( auto & nx : next) {
        //    if ( not contained_in(nx, Space))
        //      current.push_back(nx);
        //  }
        //  next.clear();
        //}
    }
    end = clock();
    //time_spent = (double)(end-begin) / CLOCKS_PER_SEC;
    time_spent = (end-begin) / CLOCKS_PER_SEC;
    cout << "Total time (in seconds) : " << time_spent << endl;
    cout << "Total memory (in MB) : " << total_memory_in_bytes()/(1024*1024) << endl;

    stats.print();

    cout << "Total time inside contains()  : " << contains_stat.get_total() << endl;
    cout << "Number of calls to contains() : " << contains_stat.get_counter() << endl;
    cout << "Max time inside contains()    : " << contains_stat.get_max() << endl;
    cout << "---------------------------------------------------------" << endl;
    cout << "Total time inside post()      : " << post_stat.get_total() << endl;
}


void model_stats::print()
{
    int te = eliminated+past_elim_from_next+past_elim_from_current+past_elim_from_space;
    cout << "Total generated states: " << total_states << endl;
    cout << "Eliminated:             " << eliminated << endl;
    cout << "Eliminated from next:   " << past_elim_from_next << endl;
    cout << "Eliminated from current:" << past_elim_from_current << endl;
    cout << "Eliminated from space:  " << past_elim_from_space << endl;
    cout << "--------------------------------------" << endl;
    cout << "Total eliminated:       " << te << endl;
    cout << "--------------------------------------" << endl;
    cout << "Final states:           " << total_states - te << endl;
}


bool Model::contained_in(const shared_ptr<Symbolic_State> &ss, const list<shared_ptr<Symbolic_State> > &lss, const list<shared_ptr<Symbolic_State> >::iterator & curr)
{
    for ( auto it = lss.begin(); it != lss.end(); it++) {
      if ( it == curr)
        continue;
      contains_stat.start();
      bool f = (*it)->contains(ss);
      contains_stat.stop();
      if (f) return true;
    }
    return false;
}

bool Model::merged_in(const shared_ptr<Symbolic_State> &ss, const list<shared_ptr<Symbolic_State> > &lss)
{
    for ( auto it = lss.begin(); it != lss.end(); it++) {
	//contains_stat.start();
	bool f = (*it)->merge(ss);
	//contains_stat.stop();
	if (f) return true;
    }
    return false;
}

bool Model::contained_in(const shared_ptr<Symbolic_State> &ss, const list<shared_ptr<Symbolic_State> > &lss)
{
    for ( auto it = lss.begin(); it != lss.end(); it++) {
	contains_stat.start();
	bool f = (*it)->contains(ss);
	contains_stat.stop();
	if (f) return true;
    }
    return false;
}

int Model::remove_included_sstates_in_a_list(const shared_ptr<Symbolic_State> &ss, list<shared_ptr<Symbolic_State> > &lss)
{
    int count = 0;
    auto it = lss.begin();
    while (it != lss.end()){
        // auto s1 = (*it)->get_signature();
        // auto s2 = ss->get_signature();
        // if (!s2.includes(s1)) {
        //     it ++;
        //     continue;
        // }
	contains_stat.start();
	bool f = ss->contains(*it); 
	contains_stat.stop();
	if (f) {
	    it = lss.erase(it);
	    count++;
	}
	else it++;
    }
    return count;
}

//int Model::remove_invalid_sstates_in_a_list(list<shared_ptr<Symbolic_State> > &lss)
//{
//    int count = 0;
//    auto it = lss.begin();
//    while (it != lss.end()){
//	if (not (*it)->is_valid()) {
//	    it = lss.erase(it);
//	    count++;
//	}
//	else it++;
//    }
//    return count;
//}

int Model::invalidate_included_sstates_in_a_list(const shared_ptr<Symbolic_State> &ss, list<shared_ptr<Symbolic_State> > &lss)
{
    int count = 0;
    auto it = lss.begin();
    while (it != lss.end()){
        // auto s1 = (*it)->get_signature();
        // auto s2 = ss->get_signature();
        // if (!s2.includes(s1)) {
        //     it ++;
        //     continue;
        // }
	contains_stat.start();
	bool f = ss->contains(*it); 
	contains_stat.stop();
	if (f) {
	    //it = lss.erase(it);
	    //count++;
            (*it)->invalidate();
	}
	it++;
    }
    return count;
}

void Model::print() const
{
    for (auto it = cvars.begin(); it != cvars.end(); it++)
    {
	if (it != cvars.begin())
	    cout << ",";
	cout << *it;
    }
    cout << ": continous;" << endl;
    for (auto it = dvars.begin(); it != dvars.end(); it++)
    {
	if (it != dvars.begin())
	    cout << ",";
	cout << it->first << " = " << it->second;
    }
    cout << ": discrete;" << endl;
    
    ////////////// Model with parameters /////////////////////////////////
    if ( parameters.size() != 0) {
        for (auto it = parameters.begin(); it != parameters.end(); it++)
        {
                if (it != parameters.begin())
                    cout << ", ";
                cout << it->min << "<=" << it->name << "<=" << it->max;
        }
        cout << ": parameters;" << endl;
    }
    /////////////////////////////////////////////////////////////////////

    for (auto it = automata.begin(); it != automata.end(); it++) {
	it->print();
    }
    cout << endl;
    cout << "init := " << endl;
    for ( auto it = automata.begin(); it != automata.end(); it++) {
	cout << "loc[" << it->get_name() << "]==" << it->get_init_location() <<"& ";
    }
    init_constraint.print();
    cout << ";" << endl;
    cout << "bad := ";
    bool first_bad = true;
    for ( auto it = automata.begin(); it != automata.end(); it++) {
	vector<Location> locations = it->get_all_locations();
	for ( auto jt = locations.begin(); jt != locations.end(); jt++) {
	    if (jt->is_bad())  {
		if (first_bad) {
		    cout << "loc[" << it->get_name() << "]==" << jt->get_name();
		    first_bad = false;
		}
		else {
		    cout << "& loc[" << it->get_name() << "]==" << jt->get_name();
		}
	    }
	}
    }
    cout << ";" << endl;
}

void Model::add_automaton(const automaton &a)
{
    automata.push_back(a);
}

void Model::set_init(const constraint &ini)
{
    init_constraint = ini;
}

void Model::add_cvar(const std::string &cv)
{
    cvars.insert(cvars.end(), cv);
}

void Model::add_dvar(const std::string &dv, int value)
{
    dvars.insert(make_pair(dv, value));
}

void Model::check_consistency() 
{
    int i = 0;
    for (auto it = automata.begin(); it != automata.end(); it++) {
	it->check_consistency(cvars, dvars);
	it->set_index(i++);
    }
}

automaton& Model::get_automaton_by_name(const std::string name)
{
    for (auto &a : automata) {
	if (a.get_name() == name) return a;
    }
    throw string("Automaton ") + name + " not found";
}

int Model::total_memory_in_bytes() const
{
    int total = 0;
    for (auto &pss : Space )
        total += pss->total_memory_in_bytes();
    return total;
}

void Model::print_log(const string fname) const
{
    ofstream of(fname);
    std::streambuf *coutbuf = std::cout.rdbuf(); 
    std::cout.rdbuf(of.rdbuf()); 

    for (auto &pss : Space)
        pss->print();
    std::cout.rdbuf(coutbuf);
}

void Model::set_sstate_type(enum SYMBOLIC_STATE_TYPE t)
{
    sstate_type = t;
}

