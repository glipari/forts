#ifndef _SSTATE_HPP_
#define _SSTATE_HPP_

#include <string>
#include <vector>
#include <ppl.hh>
#include <common.hpp>

namespace PPL = Parma_Polyhedra_Library;

class Location;
class Combined_edge;

class Symbolic_State {
    // for each automaton, the name of the corresponding location
    //std::vector<std::string> loc_names;

    // for each automaton, a pointer to the corresponding location
    std::vector<Location *> locations;
    Valuations dvars;
    PPL::C_Polyhedron cvx;
    
    PPL::C_Polyhedron invariant_cvx;


public:

    Symbolic_State(std::vector<Location *> &locations, 
		   const Valuations &dvars);

    Symbolic_State(const std::vector<std::string> &loc_names, 
		   const Valuations &dvars,
		   const PPL::C_Polyhedron &pol);

    // return true if it contains a bad state
    bool is_bad() const ; 

    void continuous_step();
    void discrete_step(Combined_edge &edges);

    PPL::C_Polyhedron get_invariant_cvx();

    std::vector<Symbolic_State> post() const;

    bool contains(const Symbolic_State &ss) const;

    bool is_empty() const; 

    int total_memory_in_bytes() const;

    void print() const;

    bool operator == (const Symbolic_State &ss) const;
};

#endif
