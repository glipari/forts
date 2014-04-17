#ifndef _SSTATE_HPP_
#define _SSTATE_HPP_

#include <string>
#include <vector>
#include <ppl.hh>
#include <common.hpp>
#include <map>
#include <memory>
#include <ostream>
#include <ppl_adapt.hpp>

namespace PPL = Parma_Polyhedra_Library;

class Location;
class Combined_edge;
class Symbolic_State;

typedef std::shared_ptr<Symbolic_State> State_ptr;
typedef std::list<State_ptr>            Space_list;
typedef std::list<State_ptr>::iterator  Space_iter;

class Signature {
    std::string str;
    unsigned active_tasks;
public :
    Signature () {}
    Signature (const std::string &s);
    const std::string& get_str() const;
    const unsigned& get_active_tasks() const;
    bool operator == (const Signature &sig) const;
    bool operator < (const Signature &sig) const;
    bool includes(const Signature &sig) const;
};

std::ostream & operator<<(std::ostream& os, const Signature &s);


class Symbolic_State {
protected:
    // for each automaton, the name of the corresponding location
    //std::vector<std::string> loc_names;

    Signature signature;
    // for each automaton, a pointer to the corresponding location
    std::vector<Location *> locations;
    Valuations dvars;
    PPL::C_Polyhedron cvx;
    
    PPL::C_Polyhedron invariant_cvx;
    
    virtual State_ptr clone() const;

public:

    Symbolic_State(std::vector<Location *> &locations, 
		   const Valuations &dvars);

    Symbolic_State(const std::vector<std::string> &loc_names, 
		   const Valuations &dvars,
		   const PPL::C_Polyhedron &pol);

    // return true if it contains a bad state
    bool is_bad() const ; 

    virtual void continuous_step();
    void discrete_step(Combined_edge &edges);

    PPL::C_Polyhedron get_invariant_cvx() const;

    virtual const PPL::C_Polyhedron get_cvx() const;

    virtual std::vector<State_ptr> post() const;

    virtual bool contains(const State_ptr  &pss) const;

    bool is_empty() const; 

    int total_memory_in_bytes() const;

    virtual void print() const;

    bool operator == (const Symbolic_State &ss) const;

    virtual bool equals(const State_ptr &pss) const;

    std::string get_loc_names() const;

    Signature get_signature() const;

    void update_signature();
};




#endif
