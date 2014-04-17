#ifndef _BOX_WIDENED_SSTATE_HPP_
#define _BOX_WIDENED_SSTATE_HPP_

#include "widened_sstate.hpp"


class Box_Widened_Symbolic_State : public Widened_Symbolic_State {
protected:

    TBox box_widened_cvx;
    virtual State_ptr clone() const;

public :
    Box_Widened_Symbolic_State(std::vector<Location *> &locations, 
		   const Valuations &dvars);

    Box_Widened_Symbolic_State(const std::vector<std::string> &loc_names, 
		   const Valuations &dvars,
		   const PPL::C_Polyhedron &pol);

    virtual bool contains(const State_ptr &pss) const;
    virtual void widen();
    //virtual void continuous_step();
    //virtual const PPL::C_Polyhedron& get_cvx() const;
    //virtual bool equals(const std::shared_ptr<Symbolic_State> &pss) const;
    virtual void print() const;
    //void widen();
};


#endif
