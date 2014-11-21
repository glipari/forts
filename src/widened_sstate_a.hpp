#ifndef _WIDENED_SSTATE_A_HPP_
#define _WIDENED_SSTATE_A_HPP_

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

class Widened_Symbolic_State_A : public Symbolic_State {
protected:
    PPL::NNC_Polyhedron widened_cvx;
    PPL::NNC_Polyhedron widened_cvx_a;
    TBox box_widened_cvx;
    TBox box_widened_cvx_a;
    virtual std::shared_ptr<Symbolic_State> clone() const;

public :
    Widened_Symbolic_State_A(std::vector<Location *> &locations, 
		   const Valuations &dvars);

    Widened_Symbolic_State_A(const std::vector<std::string> &loc_names, 
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
    virtual void widen_a();
};


#endif
