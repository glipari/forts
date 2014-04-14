#ifndef _WIDENED_SSTATE_HPP_
#define _WIDENED_SSTATE_HPP_

#include "sstate.hpp"

class Widened_Symbolic_State : public Symbolic_State {
protected:

    PPL::C_Polyhedron widened_cvx;

    virtual std::shared_ptr<Symbolic_State> clone() const;

public :

    
    Widened_Symbolic_State(std::vector<Location *> &locations, 
		   const Valuations &dvars);

    Widened_Symbolic_State(const std::vector<std::string> &loc_names, 
		   const Valuations &dvars,
		   const PPL::C_Polyhedron &pol);

    virtual bool contains(const std::shared_ptr<Symbolic_State> &pss) const;
    virtual void continuous_step();
    virtual const PPL::C_Polyhedron& get_cvx() const;
    virtual bool equals(const std::shared_ptr<Symbolic_State> &pss) const;
    virtual void print() const;

    void widen();

};


#endif
