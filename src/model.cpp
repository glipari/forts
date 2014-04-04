#include <iostream>
#include <algorithm>
#include <fstream>
#include <time.h>

#include <expression.hpp>
#include <model.hpp>

#include "combined_edge.hpp"

using namespace std;
using namespace Parma_Polyhedra_Library::IO_Operators;

Model *Model::the_instance = nullptr;

Model::Model()
{
}

// PPL::C_Polyhedron Model::get_invariant_cvx(Symbolic_State &ss)
// {
//     PPL::C_Polyhedron invariant_cvx(cvars.size());
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
}

// void Model::continuous_step(Symbolic_State &ss)
// {
//     // 1) To do time_elapse_assign
//     PPL::C_Polyhedron rates_cvx(cvars.size());
//     PPL::C_Polyhedron invariant_cvx(cvars.size());
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
void Model::discrete_step(Symbolic_State &ss, Combined_edge &edges)
{
    // PPL::C_Polyhedron guard_cvx(cvars.size());
    // PPL::C_Polyhedron ass_cvx(cvars.size());
    // Variables_Set vs;

    // for (auto &e : edges.get_edges()) {
    // 	ss.loc_names[e.get_automaton_index()] = e.get_dest();
    // 	guard_cvx.add_constraints(e.guard_to_Linear_Constraint(cvars, dvars));
    // 	Variables_Set vs2 = e.get_assignment_vars(cvars);
    // 	vs.insert(vs2.begin(), vs2.end());
    // 	// for (auto &a : e.assignments)
    // 	//     vs.insert(get_variable(a.get_var(), cvars));
    // 	ass_cvx.add_constraints(e.ass_to_Linear_Constraint(cvars, dvars));
    // }
    // ss.cvx.intersection_assign(guard_cvx);
    // ss.cvx.unconstrain(vs);
    // ss.cvx.intersection_assign(ass_cvx);
    // ss.cvx.intersection_assign(get_invariant_cvx(ss));
    ss.discrete_step(edges);
}



vector<Symbolic_State> Model::Post(const Symbolic_State& ss)
{
    // vector< vector<Edge> > v_edges;
    // vector<Symbolic_State> v_ss;
    // vector<Symbolic_State> &sstates = v_ss;
    // vector<string> synch_labels; 

    // int a_index;
    // vector<Combined_edge> edge_groups;
    // for (auto loc_it = ss.loc_names.begin(); loc_it != ss.loc_names.end(); ++loc_it, ++a_index) {
    //     a_index = loc_it - ss.loc_names.begin();
    // 	Location &l = automata[a_index].get_location_by_name(*loc_it);
    //     vector<string> new_labels = automata[a_index].get_labels();
    //     combine(edge_groups, l, new_labels, loc_it==ss.loc_names.begin());
    //     //combine(l, new_labels, edge_groups, synch_labels, loc_it==ss.loc_names.begin());
    // }
    // for ( auto it = edge_groups.begin(); it != edge_groups.end(); it++) {
    // 	Symbolic_State nss = ss;
    // 	discrete_step(nss, *it);
    // 	continuous_step(nss);
    // 	sstates.push_back(nss);
    // }

    return ss.post();
}

Symbolic_State Model::init_sstate()
{
    vector<Location *> locs;
    vector<std::string> loc_names;
    PPL::C_Polyhedron cvx(cvars.size());

    string ln="";
    for (auto it = automata.begin(); it != automata.end(); it++) {
	cout << "loc name " << it->get_init_location() << endl;
	//init.loc_names.push_back(it->get_init_location());
	loc_names.push_back(it->get_init_location());
	locs.push_back(&(it->get_location_by_name(it->get_init_location())));
	ln += it->get_init_location();
    }
    cout << "init name " << ln << endl; 
    cvx = C_Polyhedron(init_constraint.to_Linear_Constraint(cvars, dvars));
    Symbolic_State init(loc_names, dvars, cvx);
    init.print();
    init.continuous_step();
    cout << "cvx after continuous step : ";
    init.print();
    return init;
}

static bool contained_in(const Symbolic_State &ss, const list<Symbolic_State> &lss);
static void remove_included_sstates_in_a_list(const Symbolic_State &ss, list<Symbolic_State> &lss);

void Model::SpaceExplorer()
{
    clock_t begin, end;
    double time_spent;
    begin = clock();
    Symbolic_State init = init_sstate();
    list<Symbolic_State> next;
    list<Symbolic_State> current;
    current.push_back(init);
    int step = 0;
    while(true) {
	for ( auto it = current.begin(); it != current.end(); it++) {
	    vector<Symbolic_State> nsstates = Post(*it); 
	    for (auto iit = nsstates.begin(); iit != nsstates.end(); iit++) {
		if ( iit->is_empty()) continue;
		if ( iit->is_bad()) {
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
	    break;
	current.splice(current.begin(), next);
    }
    end = clock();
    time_spent = (double)(end-begin) / CLOCKS_PER_SEC;
    cout << "Total time (in seconds) : " << time_spent << endl;
    cout << "Total memory (in MB) : " << total_memory_in_bytes()/(1024*1024) << endl;
}

// bool Model::is_bad(const Symbolic_State &ss)
// {
//     for (auto it = ss.loc_names.begin(); it != ss.loc_names.end(); it++) {
// 	Location &l = automata[it - ss.loc_names.begin()].get_location_by_name(*it);
// 	if (l.is_bad())
// 	    return true;
//     }
//     return false;
// }

static bool contained_in(const Symbolic_State &ss, const list<Symbolic_State> &lss)
{
    for ( auto it = lss.begin(); it != lss.end(); it++)
	if ( it->contains(ss))
	    return true;
    return false;
}

static void remove_included_sstates_in_a_list(const Symbolic_State &ss, list<Symbolic_State> &lss)
{
    auto it = lss.begin();
    while (it != lss.end()){
	    if (ss.contains(*it)) {
	        it = lss.erase(it);
	        continue;
	    }
	    it++;
    }
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
    cvars.insert(cv);
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
    for (auto &ss : Space )
        total += ss.total_memory_in_bytes();
    return total;
}

void Model::print_log(const string fname) const
{
    ofstream of(fname);
    std::streambuf *coutbuf = std::cout.rdbuf(); 
    std::cout.rdbuf(of.rdbuf()); 

    for (auto &ss : Space)
        ss.print();
    std::cout.rdbuf(coutbuf);
}
