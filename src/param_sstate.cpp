#include "param_sstate.hpp"
#include "model.hpp"

using namespace std;
using namespace Parma_Polyhedra_Library::IO_Operators;

Param_Symbolic_State::Param_Symbolic_State(const std::vector<std::string> &loc_names, 
			       const Valuations &dv,
			       const PPL::NNC_Polyhedron &pol) : Symbolic_State(loc_names, dv, pol) {} 

//PPL::NNC_Polyhedron Param_Symbolic_State::get_invariant_cvx()
//{
//
//    PPL::NNC_Polyhedron i_cvx(dual_cvars.size());
//    for (auto p : locations) {
//        Linear_Constraint lc;
//        i_cvx.add_constraints(p->invariant_to_Linear_Constraint(dual_cvars, dual_dvars));
//    }
//    return i_cvx;
//}
    
std::shared_ptr<Symbolic_State> Param_Symbolic_State::clone() const
{
    return make_shared<Param_Symbolic_State> (*this);
}

//void Param_Symbolic_State::discrete_step(const Combined_edge &edges)
//{
//
//    PPL::NNC_Polyhedron dual_guard_cvx(dual_cvars.size());
//    PPL::NNC_Polyhedron dual_ass_cvx(dual_cvars.size()*2);
//
//    Variables_Set vs;
//    
//    auto l = locations;
//
//    for (auto &e : edges.get_edges()) {
//	    dual_guard_cvx.add_constraints(e.guard_to_Linear_Constraint(dual_cvars, dual_dvars));
//	    Variables_Set vs2 = e.get_assignment_vars(dual_cvars);
//	    vs.insert(vs2.begin(), vs2.end());
//	    dual_ass_cvx.add_constraints(e.ass_to_Linear_Constraint(dual_cvars, dual_dvars));
//    }
//    cvx.intersection_assign(dual_guard_cvx);
//
//    // This line is suspicious ...
//    dual_ass_cvx.add_constraints(cvx.constraints());
//
//    PPL::Variables_Set lower_dims;
//    for ( unsigned i = 0; i < dual_cvars.size(); i++)
//      lower_dims.insert(PPL::Variable(i));
//    dual_ass_cvx.remove_space_dimensions(lower_dims);
//
//    cvx.unconstrain(vs);
//    cvx.intersection_assign(dual_ass_cvx);
//    invariant_cvx = get_invariant_cvx();
//    cvx.intersection_assign(invariant_cvx);
//
//}

void Param_Symbolic_State::continuous_step()
{

    VariableList cvars = MODEL.get_cvars();
    PPL::NNC_Polyhedron r_cvx(cvars.size());
    
    VariableList lvars = cvars;
    for (auto p: locations) {
	    Linear_Constraint lc;
	    r_cvx.add_constraints(p->rates_to_Linear_Constraint(cvars, dvars, lvars));
    }

    for (auto &v : lvars) {
	    PPL::Variable var = get_ppl_variable(cvars, v);
	    Linear_Expr le;
        if( MODEL.is_parameter(v))
	        le += 0;
        else
	        le += 1;
	    AT_Constraint atc = (var == le);
	    r_cvx.add_constraint(atc);
    }
    cvx.time_elapse_assign(r_cvx);
    //cvx.intersection_assign(i_cvx);
    cvx.intersection_assign(invariant_cvx);
    // invariant_cvx is only used in discrete and continuous steps 
    invariant_cvx.remove_higher_space_dimensions(0);

    //if( (not cvx.is_empty()) and dual_cvx.is_empty()) {
    //    cout <<"CVX : " << cvx << endl;
    //    cout <<"Dual CVX : " << dual_cvx << endl;
    //    Symbolic_State::print();
    //    cout << endl << endl << endl << endl;
    //    print();
    //    throw("Inconsistent non-empty-cvx and empty-dual-cvx ...");
    //}
}

//const VariableList & Param_Symbolic_State::get_dual_cvars() const
//{
//    return dual_cvars;
//}
// 
//const Valuations & Param_Symbolic_State::get_dual_dvars() const
//{
//    return dual_dvars;
//}
 
//void Param_Symbolic_State::print() const
//{
//    Symbolic_State::print();
//    //cout << "Dual CVX : " << endl;
//    //cout << dual_cvx << endl;
//    //cout << "Dual discrete variables : \n";
//    //for (auto it = dual_dvars.begin(); it != dual_dvars.end(); it++) {
//    //  if (it != dual_dvars.begin())
//    //      cout << ", ";
//    //  cout << it->first << "==" << it->second;
//    //}
//    //cout << endl;
//    //cout << "Dual continuous variables : \n";
//    //for (auto it = dual_cvars.begin(); it != dual_cvars.end(); it++) {
//    //  if (it != dual_cvars.begin())
//    //      cout << ", ";
//    //  cout << *it;
//    //}
//    //cout << endl;
//}
