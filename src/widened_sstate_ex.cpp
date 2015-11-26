#include <memory>
#include <iostream>
#include "widened_sstate_ex.hpp"
#include "model.hpp"

using namespace std;
using namespace PPL::IO_Operators;

Widened_Symbolic_State_ex::Widened_Symbolic_State_ex(std::vector<Location *> &locations, 
		   const Valuations &dvars) : Symbolic_State(locations, dvars)
{
}

Widened_Symbolic_State_ex::Widened_Symbolic_State_ex(const std::vector<std::string> &loc_names, 
		   const Valuations &dvars, const PPL::NNC_Polyhedron &pol) 
            : Symbolic_State(loc_names, dvars, pol)
{
}

std::shared_ptr<Symbolic_State> Widened_Symbolic_State_ex::clone() const
{
    return make_shared<Widened_Symbolic_State_ex> (*this);
}

void Widened_Symbolic_State_ex::continuous_step()
{
    Symbolic_State::continuous_step();
}

void Widened_Symbolic_State_ex::do_something()
{
    widen();
}

void Widened_Symbolic_State_ex::widen()
{
    //widened_cvx = cvx;
    //VariableList cvars = MODEL.get_cvars();
    //int dim = cvars.size();
    //widened_cvx.add_space_dimensions_and_embed(dim);

    //PPL::Constraint_System css1;
    //PPL::Variables_Set vss;
    //for ( int i = 0; i < dim; i++) {
    //    css1.insert(PPL::Variable(i)>=PPL::Variable(i+dim));
    //    vss.insert(PPL::Variable(i));
    //}
    //widened_cvx.add_constraints(css1);

    //widened_cvx.remove_space_dimensions(vss);
    VariableList cvars = MODEL.get_cvars();
    int dim = cvars.size();
    widened_cvx = cvx;

    for ( int i = 0; i < dim/2; i++) {
      Variable v = get_ppl_variable(cvars, "p" + to_string(i+1));
      int Ti = get_valuation(dvars, "T" + to_string(i+1));
      NNC_Polyhedron tmp(cvx);

      tmp.add_constraint(v<Ti);
      if ( not tmp.is_empty()) {
        continue;
      }
      widened_cvx.unconstrain(v);
      
      //tmp.add_constraint(v>Ti);
      //if ( not tmp.is_empty()) {
      //  widened_cvx.unconstrain(v);
      //}
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
}

const PPL::NNC_Polyhedron& Widened_Symbolic_State_ex::get_cvx() const
{
    return cvx;
}


bool Widened_Symbolic_State_ex::contains(const std::shared_ptr<Symbolic_State> &pss) const
{
    if (not signature.includes(pss->get_signature())) return false;
    return widened_cvx.contains(pss->get_cvx());
}

void Widened_Symbolic_State_ex::print() const 
{
    Symbolic_State::print();
    cout << "Tasks Signature : " << signature.get_active_tasks() << endl;
    cout << "Widened cvx : \n";
    cout << widened_cvx << endl ;
}

bool Widened_Symbolic_State_ex::equals(const std::shared_ptr<Symbolic_State> &pss) const
{
    bool res = Symbolic_State::equals(pss);
    if (res) {
        res = get_cvx().contains(pss->get_cvx()) && pss->get_cvx().contains(get_cvx());
    }
    return res;
}

void Widened_Symbolic_State_ex::clear() {
  cvx.remove_higher_space_dimensions(0);
}
