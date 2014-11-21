#include <memory>
#include <iostream>
#include "widened_sstate_d.hpp"
#include "model.hpp"
#include "edge_factory.hpp"

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

    NNC_Polyhedron tmp2;
    //PPL::Variables_Set vss2;
    for ( int i = 1; i <= dim/2; i++) {
      PPL::Variable v = get_ppl_variable(cvars, "p"+to_string(i));
      tmp2 = widened_cvx;
      tmp2.add_constraint(v>=get_valuation(dvars, "T"+to_string(i)));
      if ( not tmp2.is_empty())
        //vss2.insert(v);
        widened_cvx.unconstrain(v);
    }
    //widened_cvx.unconstrain(vss2);
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


vector<shared_ptr<Symbolic_State> > Widened_Symbolic_State_D::post() const
{
    vector<shared_ptr<Symbolic_State> > sstates;
    //auto it = signature_to_combined_edges.find(signature);
    vector<Combined_edge> eg = EDGE_FACTORY.get_edges(signature, locations);
    for (auto e : eg) {
      if( not is_proceeding(e))
        continue;
        auto nss = clone();
        nss->discrete_step(e);
        nss->continuous_step();
        /** Do not forget to update the signature for the next sstate. */
        nss->update_signature();
        sstates.push_back(nss);
    }

    return sstates;
}

bool Widened_Symbolic_State_D::is_proceeding(const Combined_edge &edges) const
{
  //if( signature.get_lowest_count() == 0)
  //  return true;

  string new_str = "";
  for (auto &e : edges.get_edges()) {
    new_str += e.get_dst_location().get_name();
   // if (!found) 
   //   throw string("ERROR!!! Cannot find src location ") +
   //     e.get_src_location().get_name();
  }

  Signature sig(new_str);

  if ( new_str == "error")
    return true;

  if( sig.includes(signature))
    return true;

  //if ( sig.get_lowest_count() == 0) {
  //  cout << signature.get_str() << ", " << new_str << endl;
  //  return false;
  //}
  //else 
  if ( sig.get_highest_count() < signature.get_highest_count()) {
    //cout << signature.get_str() << ", " << new_str << endl;
    return false;
  }
  return true;
  
}
