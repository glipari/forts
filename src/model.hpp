#ifndef _MODEL_HPP_
#define _MODEL_HPP_

#include "sstate.hpp"
#include "combined_edge.hpp"

typedef constraint_node constraint;

// class Combined_edge {
// public:
//     std::vector<edge> edges;
//     std::string sync_label;
//     // The sync label set where the non empty sync_label is from
//     std::vector<std::string> sync_set;

//     Combined_edge();
    
//     std::vector<Combined_edge> combine(const edge &e, const std::vector<std::string> e_syncs);
//     bool operator == (const Combined_edge &ce) const;
// };

class model {
public:
    CVList cvars;
    DVList dvars;

    std::shared_ptr<constraint> init_constraint; 
    std::vector<automaton>      automata;

    void print();
    void check_consistency();
    void continuous_step(sstate &ss);
    void discrete_step(sstate &ss, const std::vector<edge> &edges);
    std::vector<sstate> Post(const sstate& ss);
    PPL::C_Polyhedron get_invariant_cvx(sstate &ss);
    sstate init_sstate();
    bool is_bad(const sstate &ss);
    /** The symbolic state space */
    std::list<sstate> Space;
    /** To explore Space according to breadth first search. */
    void SpaceExplorer();
};

#endif
