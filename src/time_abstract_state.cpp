#include <iostream>
#include <algorithm>
#include <vector>
#include <memory>

#include "time_abstract_state.hpp"

using namespace std;


Time_Abstract_State::Time_Abstract_State(const shared_ptr<Symbolic_State> & ss)
{
    auto s = ss;
    while (s->get_prior() != nullptr) {
        trace.push_back(s->get_incoming_edge());
        s = s->get_prior();
    }

    reverse(trace.begin(), trace.end());

}

const vector<Combined_edge>& Time_Abstract_State::get_trace() const
{
    return trace;
}

