#ifndef _TIME_ABSTRACT_STATE_HPP_
#define _TIME_ABSTRACT_STATE_HPP_

#include "sstate.hpp"

class Time_Abstract_State {
protected:
    //bool traced = false;
    std::vector<Combined_edge> trace;
public:
    Time_Abstract_State(const std::shared_ptr<Symbolic_State> & ss); 

    const std::vector<Combined_edge>& get_trace() const;
}; 


#endif
