#ifndef _PARAM_SSTATE_HPP_
#define _PARAM_SSTATE_HPP_

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

class Param_Symbolic_State : public Symbolic_State {
protected:

    virtual std::shared_ptr<Symbolic_State> clone() const;


public:


    Param_Symbolic_State(const std::vector<std::string> &loc_names, 
		   const Valuations &dvars,
		   const PPL::NNC_Polyhedron &pol);


    virtual void continuous_step();
//    virtual void discrete_step(const Combined_edge &edges);



    //const VariableList & get_dual_cvars() const;

    //const Valuations & get_dual_dvars() const;

    //virtual PPL::NNC_Polyhedron get_invariant_cvx();

//    virtual void print() const;
};

#endif
