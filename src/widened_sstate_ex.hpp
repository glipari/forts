#ifndef _WIDENED_SSTATE_EX_HPP_
#define _WIDENED_SSTATE_EX_HPP_

#include "sstate.hpp"
#include "signature.hpp"


/**
 * Widened_Symbolic_State_ex is for the extended slack time pre-order
 * relation for efficient multiprocessor G-FP schedulability analysis.
 */
class Widened_Symbolic_State_ex : public Symbolic_State {
protected:
    PPL::NNC_Polyhedron widened_cvx;
    virtual std::shared_ptr<Symbolic_State> clone() const;

public :
    Widened_Symbolic_State_ex(std::vector<Location *> &locations, 
		   const Valuations &dvars);

    Widened_Symbolic_State_ex(const std::vector<std::string> &loc_names, 
		   const Valuations &dvars,
		   const PPL::NNC_Polyhedron &pol);

    virtual bool contains(const std::shared_ptr<Symbolic_State> &pss) const;
    virtual void continuous_step();
    virtual const PPL::NNC_Polyhedron& get_cvx() const;
    virtual bool equals(const std::shared_ptr<Symbolic_State> &pss) const;
    virtual void print() const;
    virtual void widen();
    virtual void do_something();
};


#endif
