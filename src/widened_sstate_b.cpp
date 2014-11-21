#include <memory>
#include <iostream>
#include "widened_sstate_b.hpp"
#include "model.hpp"

using namespace std;
using namespace PPL::IO_Operators;

Widened_Symbolic_State_B::Widened_Symbolic_State_B(std::vector<Location *> &locations, 
		   const Valuations &dvars) : Symbolic_State(locations, dvars)
{
    //widen();
}

Widened_Symbolic_State_B::Widened_Symbolic_State_B(const std::vector<std::string> &loc_names, 
		   const Valuations &dvars, const PPL::NNC_Polyhedron &pol) 
            : Symbolic_State(loc_names, dvars, pol)
{
    //widen();
}

std::shared_ptr<Symbolic_State> Widened_Symbolic_State_B::clone() const
{
    return make_shared<Widened_Symbolic_State_B> (*this);
}

void Widened_Symbolic_State_B::continuous_step()
{
    Symbolic_State::continuous_step();
    widen();
}

void Widened_Symbolic_State_B::discrete_step(const Combined_edge &edges) {
    Symbolic_State::discrete_step(edges);
    widen_b();
}

//void Widened_Symbolic_State_B::widen()
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
void Widened_Symbolic_State_B::widen_b()
{
    widened_cvx_b = cvx;
    VariableList cvars = MODEL.get_cvars();
    int dim = cvars.size();
    widened_cvx_b.add_space_dimensions_and_embed(dim);

    PPL::Constraint_System css1;
    PPL::Variables_Set vss;
    for ( int i = 0; i < dim; i++) {
        css1.insert(PPL::Variable(i)>=PPL::Variable(i+dim));
        vss.insert(PPL::Variable(i));
    }
    widened_cvx_b.add_constraints(css1);

    widened_cvx_b.remove_space_dimensions(vss);
    //box_widened_cvx_b = TBox(widened_cvx_b, PPL::ANY_COMPLEXITY);
    box_widened_cvx_b = TBox(widened_cvx_b, PPL::POLYNOMIAL_COMPLEXITY);
}

void Widened_Symbolic_State_B::widen()
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
    //box_widened_cvx = TBox(widened_cvx, PPL::ANY_COMPLEXITY);
    box_widened_cvx = TBox(widened_cvx, PPL::POLYNOMIAL_COMPLEXITY);
}

const PPL::NNC_Polyhedron& Widened_Symbolic_State_B::get_cvx() const
{
    return widened_cvx;
}

//const PPL::NNC_Polyhedron& Widened_Symbolic_State_B::get_featured_cvx() const
//{
//    return widened_cvx;
//}

bool Widened_Symbolic_State_B::contains(const std::shared_ptr<Symbolic_State> &pss) const
{
    if (not signature.includes(pss->get_signature())) return false;
    //return widened_cvx.contains(pss->get_cvx());
    auto mypss = dynamic_pointer_cast<Widened_Symbolic_State_B>(pss); 
    if (not box_widened_cvx.contains(mypss->box_widened_cvx_b))
      return false;
//TBox(pss->get_cvx(), PPL::ANY_COMPLEXITY)))
    return widened_cvx.contains(mypss->widened_cvx_b);
}

void Widened_Symbolic_State_B::print() const 
{
    Symbolic_State::print();
    cout << "Tasks Signature : " << signature.get_active_tasks() << endl;
    cout << "Widened cvx : \n";
    cout << widened_cvx << endl ;
}

bool Widened_Symbolic_State_B::equals(const std::shared_ptr<Symbolic_State> &pss) const
{
    bool res = Symbolic_State::equals(pss);
    if (res) {
        res = get_cvx().contains(pss->get_cvx()) && pss->get_cvx().contains(get_cvx());
    }
    return res;
}

void Widened_Symbolic_State_B::clear ()
{
  widened_cvx_b.remove_higher_space_dimensions(0);
  box_widened_cvx_b.remove_higher_space_dimensions(0);
  cvx.remove_higher_space_dimensions(0);
}
