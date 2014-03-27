#ifndef _MODEL_HPP_
#define _MODEL_HPP_

#include <vector>
#include <string>

#include "sstate.hpp"
#include "automaton.hpp"

typedef constraint_node constraint;

class Combined_edge;

class model {
public:
    // continuous vars for this model
    CVList cvars;
    // discrete variables for this model
    DVList dvars;

    // the initial constraints
    constraint              init_constraint; 

    // the set of automatons
    std::vector<automaton>  automata;

    void print() const;
    void check_consistency();

    // Given a initial sstate, performs a continuous step
    void continuous_step(sstate &ss);
    // given an initial sstate and a combined edge, performs a discrete step
    void discrete_step(sstate &ss, Combined_edge &edges);

    // performs a step in the exploration of the state space
    std::vector<sstate> Post(const sstate& ss);

    // ????
    PPL::C_Polyhedron get_invariant_cvx(sstate &ss);

    // the initial state
    sstate init_sstate();

    // if the current state is bad
    bool is_bad(const sstate &ss);

    /** The symbolic state space */
    std::list<sstate> Space;

    /** 
	To explore Space according to breadth first search.  

	This is the function that performs reachability analysis.
    */
    void SpaceExplorer();
};

#endif
