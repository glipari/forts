#include <memory>
#include <iostream>
#include "widened_sstate.hpp"
#include "model.hpp"

using namespace std;
using namespace PPL::IO_Operators;

Widened_Symbolic_State::Widened_Symbolic_State(std::vector<Location *> &locations, 
		   const Valuations &dvars) : Symbolic_State(locations, dvars)
{
    //widen();
}

Widened_Symbolic_State::Widened_Symbolic_State(const std::vector<std::string> &loc_names, 
		   const Valuations &dvars, const PPL::NNC_Polyhedron &pol) 
            : Symbolic_State(loc_names, dvars, pol)
{
    //widen();
}

std::shared_ptr<Symbolic_State> Widened_Symbolic_State::clone() const
{
    return make_shared<Widened_Symbolic_State> (*this);
}

void Widened_Symbolic_State::continuous_step()
{
    Symbolic_State::continuous_step();
    //widen();
}

void Widened_Symbolic_State::do_something()
{
    widen();
}

//void Widened_Symbolic_State::widen()
//{
//    widened_cvx = cvx;
//    VariableList cvars = MODEL.get_cvars();
//    int dim = cvars.size();
//    widened_cvx.add_space_dimensions_and_embed(dim);
//
//    PPL::Constraint_System css1, css2;
//
//    for ( int i = 0; i < dim; i++) {
//        css1.insert(PPL::Variable(i)==PPL::Variable(i+dim));
//    }
//    widened_cvx.add_constraints(css1);
//
//    for ( int i = 0; i < dim; i++) {
//        widened_cvx.unconstrain(PPL::Variable(i));
//        css2.insert(PPL::Variable(i)<=PPL::Variable(i+dim));
//    }
//    widened_cvx.add_constraints(css2);
//
//    widened_cvx.remove_higher_space_dimensions(dim);
//}
void Widened_Symbolic_State::widen()
{
    widened_cvx = cvx;
    VariableList cvars = MODEL.get_cvars();
    int dim = cvars.size();
    widened_cvx.add_space_dimensions_and_embed(dim);

    PPL::Constraint_System css1;
    PPL::Variables_Set vss;
    for ( int i = 0; i < dim; i++) {
        css1.insert(PPL::Variable(i)>=PPL::Variable(i+dim));
        vss.insert(PPL::Variable(i));
    }
    widened_cvx.add_constraints(css1);

    widened_cvx.remove_space_dimensions(vss);
}

const PPL::NNC_Polyhedron& Widened_Symbolic_State::get_cvx() const
{
    //return widened_cvx;
    return cvx;
}

//const PPL::NNC_Polyhedron& Widened_Symbolic_State::get_featured_cvx() const
//{
//    return widened_cvx;
//}

bool Widened_Symbolic_State::contains(const std::shared_ptr<Symbolic_State> &pss) const
{
    if (not signature.includes(pss->get_signature())) return false;
    return widened_cvx.contains(pss->get_cvx());
}

void Widened_Symbolic_State::print() const 
{
    Symbolic_State::print();
    cout << "Tasks Signature : " << signature.get_active_tasks() << endl;
    cout << "Widened cvx : \n";
    cout << widened_cvx << endl ;
}

bool Widened_Symbolic_State::equals(const std::shared_ptr<Symbolic_State> &pss) const
{
    bool res = Symbolic_State::equals(pss);
    if (res) {
        res = get_cvx().contains(pss->get_cvx()) && pss->get_cvx().contains(get_cvx());
    }
    return res;
}

