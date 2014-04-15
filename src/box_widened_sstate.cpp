#include <memory>
#include <iostream>
#include "box_widened_sstate.hpp"
#include "model.hpp"

using namespace std;
using namespace PPL::IO_Operators;

Box_Widened_Symbolic_State::Box_Widened_Symbolic_State(std::vector<Location *> &locations, 
		   const Valuations &dvars) : Widened_Symbolic_State(locations, dvars)
{
    //widen();
}

Box_Widened_Symbolic_State::Box_Widened_Symbolic_State(const std::vector<std::string> &loc_names, 
						       const Valuations &dvars, const PPL::C_Polyhedron &pol) 
            : Widened_Symbolic_State(loc_names, dvars, pol)
{
    //widen();
}

std::shared_ptr<Symbolic_State> Box_Widened_Symbolic_State::clone() const
{
    return make_shared<Box_Widened_Symbolic_State> (*this);
}

void Box_Widened_Symbolic_State::widen()
{
    Widened_Symbolic_State::widen();
    box_widened_cvx = TBox(widened_cvx, PPL::ANY_COMPLEXITY);
}

bool Box_Widened_Symbolic_State::contains(const std::shared_ptr<Symbolic_State> &pss) const
{
    auto myptr = dynamic_pointer_cast<Box_Widened_Symbolic_State>(pss); 

    if (not signature.includes(myptr->get_signature())) return false;

    if (not box_widened_cvx.contains(myptr->box_widened_cvx))
//TBox(pss->get_cvx(), PPL::ANY_COMPLEXITY)))
        return false;
    return widened_cvx.contains(pss->get_cvx());
}

void Box_Widened_Symbolic_State::print() const 
{
    Widened_Symbolic_State::print();
    cout << "Box Widened cvx : \n";
    cout << box_widened_cvx << endl << endl;
}

bool Box_Widened_Symbolic_State::equals(const std::shared_ptr<Symbolic_State> &pss) const
{
    bool res = Widened_Symbolic_State::equals(pss);
    if (res) {
        auto myptr = dynamic_pointer_cast<Box_Widened_Symbolic_State>(pss); 
        res = (box_widened_cvx == myptr->box_widened_cvx);
    }
    return res;
}
