#ifndef _MODEL_HPP_
#define _MODEL_HPP_

#include <vector>
#include <string>
#include <memory>
#include <thread>

#include "sstate.hpp"
#include "automaton.hpp"
#include "statistics.hpp"

typedef constraint_node constraint;

class Combined_edge;

#define MODEL Model::get_instance()

struct model_stats {
    int total_states = 0; 
    int eliminated = 0; 
    int past_elim_from_next = 0;
    int past_elim_from_current = 0;
    int past_elim_from_space = 0;

    void print();
};

enum SYMBOLIC_STATE_TYPE { ORIGIN, WIDENED, BOX_WIDENED }; 

class SynchBarrier;

class Model {
    enum SYMBOLIC_STATE_TYPE sstate_type = ORIGIN;
    // continuous vars for this model
    VariableList cvars;
    // discrete variables for this model
    Valuations dvars;

    // the initial constraints
    constraint              init_constraint; 

    // the set of automatons
    std::vector<automaton>  automata;

    /** The symbolic state space */
    Space_list Space;
    Space_list current; 

    model_stats stats;
    TimeStatistic contains_stat;
    TimeStatistic post_stat;

    Model();

    static Model *the_instance; 

    bool contained_in(const State_ptr &ss, const Space_list &lss);
    int remove_included_sstates_in_a_list(const State_ptr &ss, Space_list &lss);

    struct worker_data {
	Space_list next;
	//Space_iter start;
	//Space_iter stop;
	Space_list current;
	// should add local statistics about the time and the rest;
	bool bad = false;
	model_stats stats;
	bool active = true;

	worker_data();
    };
    
    //int n_workers = 1;
    std::vector<worker_data> wdata;
    std::vector<std::thread> workers;
    void worker(SynchBarrier &barrier, unsigned n);

    void split_work(const Space_list &current, std::vector<Model::worker_data> &wdata);

public:
    Model(const Model &other) = delete;
    Model &operator=(const Model &other) = delete;

    // singleton
    static Model& get_instance(); 
    // reset the singleton
    static void reset(); 
    // print (for debugging)
    void print() const;
    // checks that everything is ok
    void check_consistency();

    // building functions 
    void add_automaton(const automaton &a); 
    void set_init(const constraint &ini); 
    void add_cvar(const std::string &cv); 
    void add_dvar(const std::string &dv, int value); 

    VariableList get_cvars() const { return cvars; }

    void discrete_step(State_ptr &pss, Combined_edge &edges);

    // Initial symbolic state
    State_ptr init_sstate();

    // throws an exception if the automaton is not found
    automaton& get_automaton_by_name(const std::string name);

    // throws an exception if the automaton is not found
    automaton& get_automaton_at(unsigned i) { return automata.at(i); }

    /** 
	To explore Space according to breadth first search. 
	This is the function that performs reachability analysis.
    */
    void SpaceExplorer();

    void SpaceExplorerParallel(int n_workers);

    Space_list &get_all_states() { return Space; }

    /** Return the memory used for symbolic states in Space. */
    int total_memory_in_bytes() const;

    /** Print the symbolic state space "Space" into a file. */
    void print_log(const std::string fname= ".log") const;

    void set_sstate_type(enum SYMBOLIC_STATE_TYPE t);

    /** sets the concurrency level. 
         if nth = 0, looks at the maximum parallelization level in hardware. 
         if nth = 1 (default), no parallelization is done
         if nth > 1, nth threads are created to carry out the internal 
	 state exploration in parallel. */
    //void set_concurrency(unsigned nth);
    //unsigned get_concurrency() const { return n_workers; }
};

#endif
