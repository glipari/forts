#ifndef _SSTATE_HPP_
#define _SSTATE_HPP_

#include <string>
#include <vector>
#include <ppl.hh>
namespace PPL = Parma_Polyhedra_Library;

class sstate {
public:
    // for each automaton, the name of the corresponding location
    std::vector<std::string> loc_names; 

    PPL::C_Polyhedron cvx;
    bool contains(const sstate &ss) const;
};

#endif
