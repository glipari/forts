#include "dual_sstate.hpp"
#include "model.hpp"

using namespace std;
using namespace Parma_Polyhedra_Library::IO_Operators;

//Dual_Symbolic_State::Dual_Symbolic_State(vector<Location*> &locations, const Valuations &dvs) :
//    Symbolic_State(locations, dvs) {
//
//    for( auto x : dvs) {
//        if(MODEL.is_parameter(x.first)) {
//            dual_cvars.insert(x.first);
//        }
//        else {
//            dual_dvars.insert(x);
//        }
//    }
//
//    for( auto x : MODEL.get_cvars()) {
//        dual_cvars.insert(x);
//    }
//}

Dual_Symbolic_State::Dual_Symbolic_State(const vector<string> &loc_names, const Valuations &dvs, const PPL::NNC_Polyhedron &pol, const PPL::NNC_Polyhedron &dual_pol) :
    Symbolic_State(loc_names, dvs, pol), dual_cvx(dual_pol) {

    for( auto x : dvs) {
        if(MODEL.is_parameter(x.first)) {
            dual_cvars.insert(x.first);
        }
        else {
            dual_dvars.insert(x);
        }
    }

    for( auto x : MODEL.get_cvars()) {
        dual_cvars.insert(x);
    }

    dual_invariant_cvx = get_dual_invariant_cvx();
}

PPL::NNC_Polyhedron Dual_Symbolic_State::get_dual_invariant_cvx()
{

    PPL::NNC_Polyhedron i_cvx(dual_cvars.size());
    for (auto p : locations) {
        Linear_Constraint lc;
        i_cvx.add_constraints(p->invariant_to_Linear_Constraint(dual_cvars, dual_dvars));
    }
    return i_cvx;
}
    
std::shared_ptr<Symbolic_State> Dual_Symbolic_State::clone() const
{
    return make_shared<Dual_Symbolic_State> (*this);
}

void Dual_Symbolic_State::discrete_step(const Combined_edge &edges)
{
    Symbolic_State::discrete_step(edges);


    PPL::NNC_Polyhedron dual_guard_cvx(dual_cvars.size());
    PPL::NNC_Polyhedron dual_ass_cvx(dual_cvars.size()*2);

    Variables_Set vs;
    
    auto l = locations;

    for (auto &e : edges.get_edges()) {
	    dual_guard_cvx.add_constraints(e.guard_to_Linear_Constraint(dual_cvars, dual_dvars));
	    Variables_Set vs2 = e.get_assignment_vars(dual_cvars);
	    vs.insert(vs2.begin(), vs2.end());
	    dual_ass_cvx.add_constraints(e.ass_to_Linear_Constraint(dual_cvars, dual_dvars));
    }
    dual_cvx.intersection_assign(dual_guard_cvx);

    dual_ass_cvx.add_constraints(dual_cvx.constraints());
    PPL::Variables_Set lower_dims;
    for ( unsigned i = 0; i < dual_cvars.size(); i++)
      lower_dims.insert(PPL::Variable(i));
    dual_ass_cvx.remove_space_dimensions(lower_dims);

    dual_cvx.unconstrain(vs);
    dual_cvx.intersection_assign(dual_ass_cvx);
    dual_invariant_cvx = get_dual_invariant_cvx();
    dual_cvx.intersection_assign(dual_invariant_cvx);

}

void Dual_Symbolic_State::continuous_step()
{
    Symbolic_State::continuous_step();


    PPL::NNC_Polyhedron r_cvx(dual_cvars.size());
    
    VariableList lvars = dual_cvars;
    for (auto p: locations) {
	    Linear_Constraint lc;
	    r_cvx.add_constraints(p->rates_to_Linear_Constraint(dual_cvars, dual_dvars, lvars));
    }

    for (auto &v : lvars) {
	    PPL::Variable var = get_ppl_variable(dual_cvars, v);
	    Linear_Expr le;
        if( MODEL.is_parameter(v))
	        le += 0;
        else
	        le += 1;
	    AT_Constraint atc = (var == le);
	    r_cvx.add_constraint(atc);
    }
    dual_cvx.time_elapse_assign(r_cvx);
    //cvx.intersection_assign(i_cvx);
    dual_cvx.intersection_assign(dual_invariant_cvx);
    // invariant_cvx is only used in discrete and continuous steps 
    dual_invariant_cvx.remove_higher_space_dimensions(0);

    if( (not cvx.is_empty()) and dual_cvx.is_empty()) {
        cout <<"CVX : " << cvx << endl;
        cout <<"Dual CVX : " << dual_cvx << endl;
        Symbolic_State::print();
        cout << endl << endl << endl << endl;
        print();
        throw("Inconsistent non-empty-cvx and empty-dual-cvx ...");
    }
}

const PPL::NNC_Polyhedron & Dual_Symbolic_State::get_dual_cvx() const
{
    return dual_cvx;
}

const VariableList & Dual_Symbolic_State::get_dual_cvars() const
{
    return dual_cvars;
}
 
const Valuations & Dual_Symbolic_State::get_dual_dvars() const
{
    return dual_dvars;
}
 
void Dual_Symbolic_State::print() const
{
    Symbolic_State::print();
    cout << "Dual CVX : " << endl;
    cout << dual_cvx << endl;
    cout << "Dual discrete variables : \n";
    for (auto it = dual_dvars.begin(); it != dual_dvars.end(); it++) {
      if (it != dual_dvars.begin())
          cout << ", ";
      cout << it->first << "==" << it->second;
    }
    cout << endl;
    cout << "Dual continuous variables : \n";
    for (auto it = dual_cvars.begin(); it != dual_cvars.end(); it++) {
      if (it != dual_cvars.begin())
          cout << ", ";
      cout << *it;
    }
    cout << endl;
}
