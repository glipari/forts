#ifndef _DBM_SSTATE_HPP_
#define _DBM_SSTATE_HPP_

#include "sstate.hpp"

class DBM_Symbolic_State : public Symbolic_State {
protected:

    PPL::BD_Shape<int> dbm_cvx;
    PPL::BD_Shape<int> invariant_dbm_cvx;
    virtual std::shared_ptr<Symbolic_State> clone() const;

    //bool accurate_cvx_is_inside = false;

public :
    DBM_Symbolic_State(std::vector<Location *> &locations, 
		   const Valuations &dvars);

    DBM_Symbolic_State(const std::vector<std::string> &loc_names, 
		   const Valuations &dvars,
		   const PPL::NNC_Polyhedron &pol);

//    DBM_Symbolic_State(const DBM_Symbolic_State &ss);

    virtual bool contains(const std::shared_ptr<Symbolic_State> &pss) const;
    virtual void continuous_step();
    virtual void discrete_step(Combined_edge &edges);
    virtual void print() const;
    virtual bool is_empty() const; 
    virtual bool is_bad(); 
    virtual int total_memory_in_bytes() const;
    //void build_accurate_cvx();
    const PPL::BD_Shape<int>& get_dbm() const;
    PPL::BD_Shape<int> get_invariant_dbm();
    //virtual const PPL::NNC_Polyhedron& get_cvx() const;
    //virtual bool equals(const std::shared_ptr<Symbolic_State> &pss) const;
    //virtual void print() const;
    //virtual void widen();
};


#endif
