#ifndef _BOX_WIDENED_SSTATE_HPP_
#define _BOX_WIDENED_SSTATE_HPP_

#include "widened_sstate.hpp"


class Box_Widened_Symbolic_State : public Widened_Symbolic_State {
protected:

    TBox box_widened_cvx;
    virtual std::shared_ptr<Symbolic_State> clone() const;

public :
    Box_Widened_Symbolic_State(std::vector<Location *> &locations, 
		   const Valuations &dvars);

    Box_Widened_Symbolic_State(const std::vector<std::string> &loc_names, 
		   const Valuations &dvars,
		   const PPL::NNC_Polyhedron &pol);

    virtual bool contains(const std::shared_ptr<Symbolic_State> &pss) const;
    virtual void widen();
    //virtual void continuous_step();
    //virtual const PPL::NNC_Polyhedron& get_cvx() const;
    virtual bool equals(const std::shared_ptr<Symbolic_State> &pss) const;
    virtual void print() const;
    virtual int64_t total_memory_in_bytes() const;
    //void widen();
};


#endif
