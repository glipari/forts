#include <iostream>
#include <algorithm>
#include <fstream>
#include <time.h>
#include <thread>


#include <expression.hpp>
#include <model.hpp>

#include "combined_edge.hpp"
#include "widened_sstate.hpp"
#include "box_widened_sstate.hpp"

#include "synch.hpp"
#include "edge_factory.hpp"

using namespace std;
using namespace Parma_Polyhedra_Library::IO_Operators;

Model *Model::the_instance = nullptr;

Model::Model() : debug_mtx{}
{
    // initializes the edge factory;
    // EDGE_FACTORY;
}

Model::worker_data::worker_data()
{
}


Model &Model::get_instance()
{
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

void Model::discrete_step(State_ptr &pss, Combined_edge &edges)
{
    pss->discrete_step(edges);
}


State_ptr Model::init_sstate()
{
    vector<Location *> locs;
    vector<std::string> loc_names;
    PPL::C_Polyhedron cvx(cvars.size());

    string ln="";
    for (auto it = automata.begin(); it != automata.end(); it++) {
	//cout << "loc name " << it->get_init_location() << endl;
	//init.loc_names.push_back(it->get_init_location());
	loc_names.push_back(it->get_init_location());
	locs.push_back(&(it->get_location_by_name(it->get_init_location())));
	ln += it->get_init_location();
    }
    //cout << "init name " << ln << endl; 
    cvx = C_Polyhedron(init_constraint.to_Linear_Constraint(cvars, dvars));

    State_ptr init;

    if (sstate_type == WIDENED)
        init = make_shared<Widened_Symbolic_State>(loc_names, dvars, cvx);
    else if (sstate_type == BOX_WIDENED)
        init = make_shared<Box_Widened_Symbolic_State>(loc_names, dvars, cvx);
    else
        init = make_shared<Symbolic_State>(loc_names, dvars, cvx);

    //init->print();
    init->continuous_step();
    //cout << "cvx after continuous step : ";
    //init->print();
    return init;
}

// i is the index of this particular worker
void Model::worker(SynchBarrier &barrier, unsigned i)
{
    barrier.synch();

    while (wdata[i].active) {
	cout << "Taking the lock ... " << endl;
	unique_lock<mutex> lck(debug_mtx);

	cout << "Worker " << i << " received " << wdata[i].current.size() << " states; " << endl;
	for (auto it = wdata[i].current.begin(); it != wdata[i].current.end(); ++it) {
	    cout << "Worker " << i << " ptr: " << (*it)->get_signature() << endl;
	    vector<State_ptr> nsstates = (*it)->post();
	    for (auto iit = nsstates.begin(); iit != nsstates.end(); iit++) {
		cout << "Worker " << i << " prod: " 
		     << (*iit)->get_signature() << endl;
		if ((*iit)->is_empty()) {
		    wdata[i].stats.eliminated++;
		    //cout << "worker " << i << " empty!" << endl;
		    continue;
		}
		if ((*iit)->is_bad()) //
		    wdata[i].bad = true;
		if (contained_in(*iit, current)) {
		    wdata[i].stats.eliminated++;
		    //cout << "worker " << i << " in current!" << endl;
		    continue;
		}
		if (contained_in(*iit, wdata[i].next)) {
		    wdata[i].stats.eliminated++;
		    //cout << "worker " << i << " in next!" << endl;
		    continue;
		}
		if (contained_in(*iit, Space)) {
		    wdata[i].stats.eliminated++;
		    //cout << "worker " << i << " in Space!" << endl;
		    continue;
		}
		//wdata[i].stats.past_elim_from_next += remove_included_sstates_in_a_list(*iit, wdata[i].next);
		wdata[i].next.push_back(*iit);
	    }
	}
	cout << "worker " << i << " produced " 
	     << wdata[i].next.size() << " states" << endl;
	lck.unlock();
	barrier.synch();
	// now next has been emptied and moved to current    
    }
}

void Model::split_work(const Space_list &current, std::vector<Model::worker_data> &wdata)
{
    const int w = std::ceil(double(current.size())/wdata.size());
    auto it = current.begin();
    for (unsigned i=0; i<wdata.size(); i++) {
	wdata[i].current.clear();
	for (unsigned k=0; it != current.end() and k<w; k++, it++) 
	    wdata[i].current.push_back(*it);
    }
}


void Model::SpaceExplorerParallel(int n_workers)
{
    clock_t begin, end;
    double time_spent;
    begin = clock();
    State_ptr init = init_sstate();
    SynchBarrier barrier(n_workers);
    int step = 0; 
    
    current.clear();
    current.push_back(init);

    // init all threads;
    wdata.clear();
    workers.clear();

    using namespace std::placeholders;

    auto tf = std::bind(&Model::worker, this, _1, _2);

    for (int i=0; i<n_workers; ++i) {
	wdata.push_back(worker_data{});
	workers.push_back(thread(tf, ref(barrier), i));
    }

    // now all workers are ready to start

    cout << "Concurrency level: " << n_workers << endl;

    bool bad_found = false;
    while (not bad_found) {
	cout << "All pointers: " << endl;
	for (auto it = std::begin(current); it != std::end(current); ++it) {
	    cout << (*it)->get_signature() << endl;
	}
	cout << "----------" << endl;

	split_work(current, wdata);
	// // split work
	// auto v = split(std::begin(current), std::end(current), 
	// 	       current.size(), n_workers);
	// assert(v.size() == n_workers);
	// for (int i=0; i<n_workers; ++i) {
	//     wdata[i].current.clear();
	//     cout << "now copying in current" << endl;
	//     for (auto it = v[i].first; it != v[i].second; ++it) 
	// 	current.push_back(*it);
	// }
	
	cout << "splitted" << endl;

	barrier.start();

	// all work is done in the parallel threads
	barrier.waitForAll();

	// move all results
	Space.splice(Space.begin(), current);
	assert(current.size() == 0);
	for (int i=0; i<n_workers; ++i) {
	    current.splice(current.begin(), wdata[i].next);
	    assert(wdata[i].next.size() == 0);
	}
	
	cout << "-----------------------------" << endl;
	cout << "Step : " << ++step << endl;
	cout << "Number of passed states : " << Space.size() << endl;
	cout << "Number of generated states : " << current.size() << endl;
	cout << "-----------------------------" << endl;
	
	if (current.size() == 0) break;
	for (int i=0; i<n_workers; ++i) {
	    bad_found = bad_found or wdata[i].bad;
	}
    }
    end = clock();
    // clean exit of the threads:
    for (int i=0; i<n_workers; i++) 
	wdata[i].active = false;
    barrier.start();
    for (int i=0; i<n_workers; i++) 
	workers[i].join();
    
    cout << "All threads correctly ended" << endl;

    time_spent = (double)(end-begin) / CLOCKS_PER_SEC;
    cout << "Total time (in seconds) : " << time_spent << endl;
    cout << "Total memory (in MB) : " << total_memory_in_bytes()/(1024*1024) 
	 << endl;
}


void Model::SpaceExplorer()
{
    clock_t begin, end;
    double time_spent;
    begin = clock();
    State_ptr init = init_sstate();
    Space_list next; 
    //Space_list current; 
    current.clear();
    current.push_back(init);
    int step = 0; 

    stats = model_stats{};

    while(true) {
	for ( auto it = current.begin(); it != current.end(); it++) {
	    post_stat.start();
	    vector<State_ptr> nsstates = (*it)->post(); //Post(*it); 
	    post_stat.stop();
	    stats.total_states += nsstates.size();

	    for (auto iit = nsstates.begin(); iit != nsstates.end(); iit++) {
		if ( (*iit)->is_empty()) {
		    stats.eliminated++;
		    continue;
		}
		if ( (*iit)->is_bad()) {
		    throw ("A bad location is reached ... ");
                }
		if ( contained_in(*iit, current)) {
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
		stats.past_elim_from_next += remove_included_sstates_in_a_list(*iit, next);
		//stats.past_elim_from_current += remove_included_sstates_in_a_list(*iit, current);
		//stats.past_elim_from_space += remove_included_sstates_in_a_list(*iit, Space);
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


bool Model::contained_in(const State_ptr &ss, const Space_list &lss)
{
    for ( auto it = lss.begin(); it != lss.end(); it++) {
        // auto s1 = (*it)->get_signature();
        // auto s2 = ss->get_signature();
        // if (!s1.includes(s2))
        //     continue;
	//contains_stat.start();
	bool f = (*it)->contains(ss);
	//contains_stat.stop();
	if (f) return true;
    }
    return false;
}

int Model::remove_included_sstates_in_a_list(const State_ptr &ss, Space_list &lss)
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
	//contains_stat.start();
	bool f = ss->contains(*it); 
	//contains_stat.stop();
	if (f) {
	    it = lss.erase(it);
	    count++;
	}
	else it++;
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

// void Model::set_concurrency(unsigned nth)
// {
//     if (nth == 0) n_workers = std::thread::hardware_concurrency();
//     else n_workers = nth;
//     // TODO create all threads?     
// }
