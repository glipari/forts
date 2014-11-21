#include <memory>
#include <iostream>
#include "widened_sstate_dec.hpp"
#include "model.hpp"

using namespace std;
using namespace PPL::IO_Operators;

Widened_Symbolic_State_DEC::Widened_Symbolic_State_DEC(std::vector<Location *> &locations, 
		   const Valuations &dvars) : Symbolic_State(locations, dvars)
{
    //widen();
}

Widened_Symbolic_State_DEC::Widened_Symbolic_State_DEC(const std::vector<std::string> &loc_names, 
		   const Valuations &dvars, const PPL::NNC_Polyhedron &pol) 
            : Symbolic_State(loc_names, dvars, pol)
{
    //widen();
}

std::shared_ptr<Symbolic_State> Widened_Symbolic_State_DEC::clone() const
{
    return make_shared<Widened_Symbolic_State_DEC> (*this);
}

void Widened_Symbolic_State_DEC::continuous_step()
{
    Symbolic_State::continuous_step();
    PPL::Variable v = get_ppl_variable(MODEL.get_cvars(), "DI");
    int dec_t = get_valuation(dvars, "DEC_T");
    cvx.add_constraint(v<=dec_t);
    //PPL::Variable v = get_ppl_variable(MODEL.cvars, "dec_t");
    //int dec_t = get_valuation(dvars, "dec_t");
    //cvx.add_constraint(v<=dec_t);
    widen();
}

void Widened_Symbolic_State_DEC::discrete_step(const Combined_edge &edges)
{
    Symbolic_State::discrete_step(edges);
    PPL::Variable v = get_ppl_variable(MODEL.get_cvars(), "DI");
    int dec_t = get_valuation(dvars, "DEC_T");
    cvx.add_constraint(v<=dec_t);
}

//void Widened_Symbolic_State_DEC::widen()
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
void Widened_Symbolic_State_DEC::widen()
{
    widened_cvx = cvx;
    VariableList cvars = MODEL.get_cvars();
    int dim = cvars.size()-1;
    widened_cvx.remove_higher_space_dimensions(dim);
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

const PPL::NNC_Polyhedron& Widened_Symbolic_State_DEC::get_cvx() const
{
    return widened_cvx;
}

//const PPL::NNC_Polyhedron& Widened_Symbolic_State_DEC::get_featured_cvx() const
//{
//    return widened_cvx;
//}

bool Widened_Symbolic_State_DEC::contains(const std::shared_ptr<Symbolic_State> &pss) const
{
    if (not signature.includes(pss->get_signature())) return false;
    return widened_cvx.contains(pss->get_cvx());
    //PPL::Variable v = get_ppl_variable(MODEL.get_cvars(), "DI");
    //NNC_Polyhedron c1 = widened_cvx;
    //NNC_Polyhedron c2 = pss->get_cvx();
    //c1.unconstrain(v);
    //c2.unconstrain(v);
    //return c1.contains(c2);
}

void Widened_Symbolic_State_DEC::print() const 
{
    Symbolic_State::print();
    cout << "Tasks Signature : " << signature.get_active_tasks() << endl;
    cout << "Widened cvx : \n";
    cout << widened_cvx << endl ;
}

bool Widened_Symbolic_State_DEC::equals(const std::shared_ptr<Symbolic_State> &pss) const
{
    bool res = Symbolic_State::equals(pss);
    if (res) {
        res = get_cvx().contains(pss->get_cvx()) && pss->get_cvx().contains(get_cvx());
    }
    return res;
}

