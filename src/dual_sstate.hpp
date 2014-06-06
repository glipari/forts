#ifndef _DUAL_SSTATE_HPP_
#define _DUAL_SSTATE_HPP_

//#include <string>
//#include <vector>
//#include <ppl.hh>
//#include <common.hpp>
//#include <map>
//#include <memory>
//#include <ppl_adapt.hpp>
//
//#include "signature.hpp"
//#include "combined_edge.hpp"

#include "sstate.hpp"

namespace PPL = Parma_Polyhedra_Library;

//class Location;
//class Combined_edge;

class Dual_Symbolic_State : public Symbolic_State {
protected:

//    PPL::NNC_Polyhedron cvx;

//    PPL::NNC_Polyhedron invariant_cvx;
    
    PPL::NNC_Polyhedron dual_cvx;

    PPL::NNC_Polyhedron dual_invariant_cvx;

    Valuations dual_dvars;

    VariableList dual_cvars;
    
    virtual std::shared_ptr<Symbolic_State> clone() const;

    PPL::NNC_Polyhedron get_dual_invariant_cvx();

public:

    //Dual_Symbolic_State(std::vector<Location *> &locations, 
	//	   const Valuations &dvars);

    Dual_Symbolic_State(const std::vector<std::string> &loc_names, 
		   const Valuations &dvars,
		   const PPL::NNC_Polyhedron &pol,
		   const PPL::NNC_Polyhedron &dual_pol);

    //Dual_Symbolic_State();

    virtual void continuous_step();
    virtual void discrete_step(const Combined_edge &edges);


    const PPL::NNC_Polyhedron & get_dual_cvx() const;

    const VariableList & get_dual_cvars() const;

    const Valuations & get_dual_dvars() const;

    virtual void print() const;
};

#endif
