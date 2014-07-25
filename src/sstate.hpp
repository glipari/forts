#ifndef _SSTATE_HPP_
#define _SSTATE_HPP_

#include <string>
#include <vector>
#include <ppl.hh>
#include <common.hpp>
#include <map>
#include <memory>
#include <ppl_adapt.hpp>

#include "signature.hpp"
#include "combined_edge.hpp"

namespace PPL = Parma_Polyhedra_Library;

class Location;
//class Combined_edge;

class Symbolic_State {
protected:
    // for each automaton, the name of the corresponding location
    //std::vector<std::string> loc_names;

    Signature signature;
    // for each automaton, a pointer to the corresponding location
    std::vector<Location *> locations;
    Valuations dvars;
    PPL::NNC_Polyhedron cvx;
    
    PPL::NNC_Polyhedron invariant_cvx;
    
    virtual std::shared_ptr<Symbolic_State> clone() const;

    // keep a track of ancester of current state : prior ===incoming_edge===>
    std::shared_ptr<Symbolic_State> prior;
    Combined_edge incoming_edge;

    bool valid = true;
public:

    Symbolic_State(std::vector<Location *> &locations, 
		   const Valuations &dvars);

    Symbolic_State(const std::vector<std::string> &loc_names, 
		   const Valuations &dvars,
		   const PPL::NNC_Polyhedron &pol);

    Symbolic_State();

    // return true if it contains a bad state
    virtual bool is_bad() const ; 

    virtual void continuous_step();
    virtual void discrete_step(const Combined_edge &edges);

    virtual PPL::NNC_Polyhedron get_invariant_cvx();

    virtual const PPL::NNC_Polyhedron& get_cvx() const;

    virtual std::vector<std::shared_ptr<Symbolic_State> > post() const;

    virtual bool contains(const std::shared_ptr<Symbolic_State> &pss) const;

    virtual bool is_empty() const; 

    virtual int total_memory_in_bytes() const;

    virtual void print() const;

    bool operator == (const Symbolic_State &ss) const;

    virtual bool equals(const std::shared_ptr<Symbolic_State> &pss) const;

    std::string get_loc_names() const;

    Signature get_signature() const;

    virtual void update_signature();


    void mark_prior(std::shared_ptr<Symbolic_State> p);
    std::shared_ptr<Symbolic_State> get_prior() const;
    Combined_edge get_incoming_edge() const;
    const std::vector<Location *>& get_locations() const;
    bool no_outgoings() const;

    virtual void refine_cvx(const PPL::NNC_Polyhedron &poly);
    bool is_valid() const;
    void invalidate();
};

#endif
