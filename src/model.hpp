#ifndef _MODEL_HPP_
#define _MODEL_HPP_

#include <vector>
#include <string>
#include <memory>

#include "sstate.hpp"
#include "automaton.hpp"

typedef constraint_node constraint;

class Combined_edge;

#define MODEL Model::get_instance()

class Model {
    bool widened = false;
    // continuous vars for this model
    VariableList cvars;
    // discrete variables for this model
    Valuations dvars;

    // the initial constraints
    constraint              init_constraint; 

    // the set of automatons
    std::vector<automaton>  automata;

    /** The symbolic state space */
    std::list<std::shared_ptr<Symbolic_State> > Space;

    Model();

    static Model *the_instance; 

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

    // TBM: Given a initial sstate, performs a continuous step
    // void continuous_step(Symbolic_State &ss);
    // TBM: given an initial sstate and a combined edge, performs a discrete step
    //void discrete_step(Symbolic_State &ss, Combined_edge &edges);
    void discrete_step(std::shared_ptr<Symbolic_State> &pss, Combined_edge &edges);

    // Maybe will become private:
    // performs a step in the exploration of the state space
    //std::vector<Symbolic_State> Post(const Symbolic_State& ss);
    std::vector<std::shared_ptr<Symbolic_State> > Post(const std::shared_ptr<Symbolic_State>& pss);

    // TBM 
    // PPL::C_Polyhedron get_invariant_cvx(Symbolic_State &ss);

    // Initial symbolic state
    std::shared_ptr<Symbolic_State> init_sstate();

    // TBM: if the current state is bad
    //bool is_bad(const Symbolic_State &ss);

    // throws an exception if the automaton is not found
    automaton& get_automaton_by_name(const std::string name);

    // throws an exception if the automaton is not found
    automaton& get_automaton_at(unsigned i) { return automata.at(i); }

    /** 
	To explore Space according to breadth first search. 
	This is the function that performs reachability analysis.
    */
    void SpaceExplorer();

    std::list<std::shared_ptr<Symbolic_State> > &get_all_states() { return Space; }


    /** Return the meomey used for symbolic states in Space. */
    int total_memory_in_bytes() const;

    /** Print the symbolic state space "Space" into a file. */
    void print_log(const std::string fname= ".log") const;

    void set_widened();
};

#endif
