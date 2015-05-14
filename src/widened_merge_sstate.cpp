#include <memory>
#include <iostream>
#include "widened_merge_sstate.hpp"
#include "model.hpp"

using namespace std;
using namespace PPL::IO_Operators;

Widened_Merge_Symbolic_State::Widened_Merge_Symbolic_State(std::vector<Location *> &locations, 
		   const Valuations &dvars) : Symbolic_State(locations, dvars)
{
}

Widened_Merge_Symbolic_State::Widened_Merge_Symbolic_State(const std::vector<std::string> &loc_names, 
		   const Valuations &dvars, const PPL::NNC_Polyhedron &pol) 
            : Symbolic_State(loc_names, dvars, pol)
{
}

std::shared_ptr<Symbolic_State> Widened_Merge_Symbolic_State::clone() const
{
    return make_shared<Widened_Merge_Symbolic_State> (*this);
}

void Widened_Merge_Symbolic_State::continuous_step()
{
    Symbolic_State::continuous_step();
}

void Widened_Merge_Symbolic_State::do_something()
{
    widen();
}


void Widened_Merge_Symbolic_State::widen()
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

const PPL::NNC_Polyhedron& Widened_Merge_Symbolic_State::get_cvx() const
{
    return cvx;
}

bool Widened_Merge_Symbolic_State::contains(const std::shared_ptr<Symbolic_State> &pss) const
{
    //if (not signature.includes(pss->get_signature())) return false;
    if (not (signature == pss->get_signature())) return false;
    return widened_cvx.contains(pss->get_cvx());
}

//bool Widened_Merge_Symbolic_State::merge(const std::shared_ptr<Symbolic_State> &pss) 
//{
//  if (not (pss->get_signature() == signature)) return false;
//  auto myptr = dynamic_pointer_cast<Widened_Merge_Symbolic_State>(pss); 
//  NNC_Polyhedron w1 = cvx, w2 = myptr->cvx;
//  w1.poly_hull_assign(w2);
//  NNC_Polyhedron com = w1;
//  w1.poly_difference_assign(cvx);
//  if ( w2.contains(w1)) {
//    cvx = com;
//    return true;
//  }
//  else return false;    
//}
bool Widened_Merge_Symbolic_State::merge(const std::shared_ptr<Symbolic_State> &pss) 
{
  if ( merge_flag > 0 ) return false;
  if (not (pss->get_signature() == signature)) return false;
  //cout << "merge ... " ;
  auto myptr = dynamic_pointer_cast<Widened_Merge_Symbolic_State>(pss); 
  myptr->merge_flag ++;
  NNC_Polyhedron w1 = cvx, w2 = myptr->cvx;
  w1.poly_hull_assign(w2);

  auto & css = w2.constraints();
  for ( auto it = css.begin(); it != css.end(); it++)
  {
    Linear_Expression e;
    for ( dimension_type i = it->space_dimension(); i -- > 0;) 
      e += it->coefficient(Variable(i)) * Variable(i);
    e += it->inhomogeneous_term();
    Constraint c;
    if ( it->is_inequality()) {
      c = it->is_strict_inequality() ? (e<=0) : (e<0);
      NNC_Polyhedron com = w1;
      com.add_constraint(c);
      //if ( not cvx.contains(com)) {cout << " done1! " << endl; return false;}
      if ( not cvx.contains(com)) {return false;}
    }
    else {
      c = (e < 0);
      NNC_Polyhedron com = w1;
      com.add_constraint(c);
      //if ( not cvx.contains(com)) {cout << " done2! " << endl; return false;}
      if ( not cvx.contains(com)) {return false;}

      c = (e > 0);
      com = w1;
      com.add_constraint(c);
      //if ( not cvx.contains(com)) {cout << " done3! " << endl; return false;}
      if ( not cvx.contains(com)) {return false;}
    }
  }
  cvx = w1;
  widen();
  //cout << " done4!" << endl;
  return true;
}

void Widened_Merge_Symbolic_State::print() const 
{
    Symbolic_State::print();
    cout << "Tasks Signature : " << signature.get_active_tasks() << endl;
    cout << "Widened cvx : \n";
    cout << widened_cvx << endl ;
}

bool Widened_Merge_Symbolic_State::equals(const std::shared_ptr<Symbolic_State> &pss) const
{
    bool res = Symbolic_State::equals(pss);
    if (res) {
        res = get_cvx().contains(pss->get_cvx()) && pss->get_cvx().contains(get_cvx());
    }
    return res;
}

