#ifndef _WIDENED_SSTATE_DEC_HPP_
#define _WIDENED_SSTATE_DEC_HPP_

#include "sstate.hpp"
#include "signature.hpp"

class Widened_Symbolic_State_DEC : public Symbolic_State {
protected:
    PPL::NNC_Polyhedron widened_cvx;
    virtual std::shared_ptr<Symbolic_State> clone() const;

public :
    Widened_Symbolic_State_DEC(std::vector<Location *> &locations, 
		   const Valuations &dvars);

    Widened_Symbolic_State_DEC(const std::vector<std::string> &loc_names, 
		   const Valuations &dvars,
		   const PPL::NNC_Polyhedron &pol);

    virtual bool contains(const std::shared_ptr<Symbolic_State> &pss) const;
    virtual void continuous_step();
    virtual void discrete_step(const Combined_edge &edges);
    virtual const PPL::NNC_Polyhedron& get_cvx() const;
    virtual bool equals(const std::shared_ptr<Symbolic_State> &pss) const;
    virtual void print() const;
    virtual void widen();
};


#endif
