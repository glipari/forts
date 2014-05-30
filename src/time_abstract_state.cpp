#include <iostream>
#include <algorithm>
#include <vector>
#include <memory>

#include "time_abstract_state.hpp"

using namespace std;

Trace::Trace() {}

Trace::Trace(const vector<Combined_edge> &v) : ces(v) {}

const vector<Combined_edge> & Trace::get_combined_edges() const
{
    return ces;
}

bool Trace::operator == (const Trace &tr) const 
{
    if( ces.size() != tr.ces.size())
        return false;
    for( int i = 0; i < ces.size(); i++) {
        if( not (ces[i] == tr.ces[i]))
            return false;
    }
    return true;
}

void Trace::print_trace() const {
    bool begin = true;
    for( auto &x: ces) {
        string name = "";
        for(auto &y: x.get_locations()) {
            name += y->get_name();
        }
        if(begin)
            begin = false;
        else
            cout << "==>";
        cout << name;
    }
    auto &x = ces.back();
    string name = "";
    for(auto &y:x.get_edges())
        name += y.get_dest();
    cout << name;
    cout << endl;
}


Time_Abstract_State::Time_Abstract_State(const shared_ptr<Symbolic_State> & ss)
{
    vector<Combined_edge> ces;
    auto s = ss;
    while (s->get_prior() != nullptr) {
        ces.push_back(s->get_incoming_edge());
        s = s->get_prior();
    }

    reverse(ces.begin(), ces.end());

    trace = Trace(ces);

}

const Trace& Time_Abstract_State::get_trace() const
{
    return trace;
}

void Time_Abstract_State::print() const {
    trace.print_trace();
}

void Time_Abstract_State::set_cvx(const PPL::NNC_Polyhedron &p)
{
    cvx = p;
}

const PPL::NNC_Polyhedron& Time_Abstract_State::get_cvx() const
{
    return cvx;
}
