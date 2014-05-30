#ifndef _TIME_ABSTRACT_STATE_HPP_
#define _TIME_ABSTRACT_STATE_HPP_

#include "sstate.hpp"

struct Trace {
    std::vector<Combined_edge> ces;
    bool operator == (const Trace& tr) const;
    Trace();
    Trace(const std::vector<Combined_edge> &ces);
    const std::vector<Combined_edge> & get_combined_edges() const;
    void print_trace() const;
    //void push_back(const Combined_edge &ce);
};

class Time_Abstract_State {
protected:
    //bool traced = false;
    //std::vector<Combined_edge> trace;
    Trace trace;
    PPL::NNC_Polyhedron cvx;
public:
    Time_Abstract_State(const std::shared_ptr<Symbolic_State> & ss); 

    const Trace& get_trace() const;

    void set_cvx(const PPL::NNC_Polyhedron &p);

    const PPL::NNC_Polyhedron& get_cvx() const;

    void print() const;
}; 


#endif
