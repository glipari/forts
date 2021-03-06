#ifndef _WIDENED_MERGE_SSTATE_HPP_
#define _WIDENED_MERGE_SSTATE_HPP_

#include "sstate.hpp"
#include "signature.hpp"

class Widened_Merge_Symbolic_State : public Symbolic_State {
protected:
  int merge_flag = 0;
    PPL::NNC_Polyhedron widened_cvx;
    virtual std::shared_ptr<Symbolic_State> clone() const;

public :
    Widened_Merge_Symbolic_State(std::vector<Location *> &locations, 
		   const Valuations &dvars);

    Widened_Merge_Symbolic_State(const std::vector<std::string> &loc_names, 
		   const Valuations &dvars,
		   const PPL::NNC_Polyhedron &pol);

    virtual bool contains(const std::shared_ptr<Symbolic_State> &pss) const;
    virtual void continuous_step();
    virtual const PPL::NNC_Polyhedron& get_cvx() const;
    virtual bool equals(const std::shared_ptr<Symbolic_State> &pss) const;
    virtual void print() const;
    virtual void widen();
    virtual void do_something();
    virtual bool merge(const std::shared_ptr<Symbolic_State> &pss); 
    virtual void clear();
    bool divide(const PPL::NNC_Polyhedron& co, const PPL::NNC_Polyhedron& w1, const PPL::NNC_Polyhedron& w2) const;
};


#endif
