#ifndef _MODEL_HPP_
#define _MODEL_HPP_

#include <vector>
#include <string>

#include "sstate.hpp"
#include "automaton.hpp"

typedef constraint_node constraint;

class Combined_edge;

#define MODEL Model::get_instance()

class Model {
    // continuous vars for this model
    VariableList cvars;
    // discrete variables for this model
    Valuations dvars;

    // the initial constraints
    constraint              init_constraint; 

    // the set of automatons
    std::vector<automaton>  automata;

    /** The symbolic state space */
    std::list<Symbolic_State> Space;

    Model();

    static Model *the_instance; 

public:
    Model(const Model &other) = delete;
    Model &operator=(const Model &other) = delete;

    static Model& get_instance(); 

    static void reset(); 

    void print() const;
    void check_consistency();

    void add_automaton(const automaton &a); 
    void set_init(const constraint &ini); 
    void add_cvar(const std::string &cv); 
    void add_dvar(const std::string &dv, int value); 

    // Given a initial sstate, performs a continuous step
    void continuous_step(Symbolic_State &ss);
    // given an initial sstate and a combined edge, performs a discrete step
    void discrete_step(Symbolic_State &ss, Combined_edge &edges);

    // performs a step in the exploration of the state space
    std::vector<Symbolic_State> Post(const Symbolic_State& ss);

    // ????
    PPL::C_Polyhedron get_invariant_cvx(Symbolic_State &ss);

    // the initial state
    Symbolic_State init_sstate();

    // if the current state is bad
    bool is_bad(const Symbolic_State &ss);

    // throws an exception if the automaton is not found
    automaton& get_automaton_by_name(const std::string name);
    /** 
	To explore Space according to breadth first search. 
	This is the function that performs reachability analysis.
    */
    void SpaceExplorer();
};

#endif
