#include <memory>
#include <iostream>
#include "widened_sstate_d.hpp"
#include "model.hpp"

using namespace std;
using namespace PPL::IO_Operators;

Widened_Symbolic_State_D::Widened_Symbolic_State_D(std::vector<Location *> &locations, 
		   const Valuations &dvars) : Symbolic_State(locations, dvars)
{
    //widen();
}

Widened_Symbolic_State_D::Widened_Symbolic_State_D(const std::vector<std::string> &loc_names, 
		   const Valuations &dvars, const PPL::NNC_Polyhedron &pol) 
            : Symbolic_State(loc_names, dvars, pol)
{
    //widen();
}

std::shared_ptr<Symbolic_State> Widened_Symbolic_State_D::clone() const
{
    return make_shared<Widened_Symbolic_State_D> (*this);
}

void Widened_Symbolic_State_D::continuous_step()
{
    Symbolic_State::continuous_step();
    //widen();
}

void Widened_Symbolic_State_D::do_something()
{
    widen();
}

//void Widened_Symbolic_State_D::widen()
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
void Widened_Symbolic_State_D::widen()
{
    VariableList cvars = MODEL.get_cvars();
    int dim = cvars.size();
    widened_cvx = cvx;

    for ( int i = 0; i < dim/2; i++) {
      Variable v = get_ppl_variable(cvars, "p" + to_string(i+1));
      int Ti = get_valuation(dvars, "T" + to_string(i+1));
      NNC_Polyhedron tmp1(cvx), tmp2(cvx);
      tmp1.add_constraint(v<=Ti);
      if ( not tmp1.is_empty())
        continue;
      tmp2.add_constraint(v>Ti);
      if ( not tmp2.is_empty()) {
        widened_cvx.unconstrain(v);
        //cout << "p" + to_string(i+1) << endl;
      }
    }


    widened_cvx.add_space_dimensions_and_embed(dim);

    PPL::Constraint_System css1;
    PPL::Variables_Set vss;
    for ( int i = 0; i < dim; i++) {
        css1.insert(PPL::Variable(i)>=PPL::Variable(i+dim));
        vss.insert(PPL::Variable(i));
    }
    widened_cvx.add_constraints(css1);

    widened_cvx.remove_space_dimensions(vss);

    ////////////// To further optimize the widen() operator //////
    ///for ( int i = 0; i < dim/2; i++) {
    ///  Variable v = get_ppl_variable(cvars, "p" + to_string(i+1));
    ///  int Ti = get_valuation(dvars, "T" + to_string(i+1));
    ///  NNC_Polyhedron tmp(widened_cvx);
    ///  tmp.add_constraint(v>Ti+1);
    ///  if ( not tmp.is_empty())
    ///    widened_cvx.unconstrain(v);
    ///}
    //cout << endl;
    //for ( auto x : dvars) 
    //  cout << x.first << ", " << x.second << endl;
    //cout << endl;


}

const PPL::NNC_Polyhedron& Widened_Symbolic_State_D::get_cvx() const
{
    //return widened_cvx;
    return cvx;
}

//const PPL::NNC_Polyhedron& Widened_Symbolic_State_D::get_featured_cvx() const
//{
//    return widened_cvx;
//}

bool Widened_Symbolic_State_D::contains(const std::shared_ptr<Symbolic_State> &pss) const
{
    if (not signature.includes(pss->get_signature())) return false;
    return widened_cvx.contains(pss->get_cvx());
}

void Widened_Symbolic_State_D::print() const 
{
    Symbolic_State::print();
    cout << "Tasks Signature : " << signature.get_active_tasks() << endl;
    cout << "Widened cvx : \n";
    cout << widened_cvx << endl ;
}

bool Widened_Symbolic_State_D::equals(const std::shared_ptr<Symbolic_State> &pss) const
{
    bool res = Symbolic_State::equals(pss);
    if (res) {
        res = get_cvx().contains(pss->get_cvx()) && pss->get_cvx().contains(get_cvx());
    }
    return res;
}

