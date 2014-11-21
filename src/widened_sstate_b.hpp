#ifndef _WIDENED_SSTATE_B_HPP_
#define _WIDENED_SSTATE_B_HPP_

#include "sstate.hpp"
#include "signature.hpp"

//class Tasks_Signature : public Signature{
//protected:
//    unsigned active_tasks;
//public:
//    //Signature () {}
//    Tasks_Signature (const std::string &s);
//    const unsigned& get_active_tasks() const;
//    bool operator == (const Tasks_Signature &sig) const;
//    bool includes(const Tasks_Signature &sig) const;
//    
//};

class Widened_Symbolic_State_B : public Symbolic_State {
protected:
    PPL::NNC_Polyhedron widened_cvx;
    PPL::NNC_Polyhedron widened_cvx_b;
    TBox box_widened_cvx;
    TBox box_widened_cvx_b;
    virtual std::shared_ptr<Symbolic_State> clone() const;

public :
    Widened_Symbolic_State_B(std::vector<Location *> &locations, 
		   const Valuations &dvars);

    Widened_Symbolic_State_B(const std::vector<std::string> &loc_names, 
		   const Valuations &dvars,
		   const PPL::NNC_Polyhedron &pol);

    virtual bool contains(const std::shared_ptr<Symbolic_State> &pss) const;
    virtual void continuous_step();
    virtual void discrete_step(const Combined_edge &edges);
    virtual const PPL::NNC_Polyhedron& get_cvx() const;
    virtual bool equals(const std::shared_ptr<Symbolic_State> &pss) const;
    virtual void print() const;
    virtual void widen();
    virtual void clear();
    virtual void widen_b();
};


#endif
