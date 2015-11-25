#include <memory>
#include <iostream>
#include "oct_sstate.hpp"
#include "model.hpp"
#include "automaton.hpp"
#include "combined_edge.hpp"

using namespace std;
using namespace PPL::IO_Operators;

OCT_Symbolic_State::OCT_Symbolic_State(std::vector<Location *> &locations, 
		   const Valuations &dvars) : Symbolic_State(locations, dvars)
{
    oct_cvx = PPL::Octagonal_Shape<int>(cvx, PPL::ANY_COMPLEXITY);
    invariant_oct_cvx = get_invariant_oct();
    //invariant_oct_cvx = PPL::Octagonal_Shape<int>(invariant_cvx, PPL::ANY_COMPLEXITY);
    cvx.remove_higher_space_dimensions(0);
    invariant_cvx.remove_higher_space_dimensions(0);
    //accurate_cvx_is_inside = false;
}

OCT_Symbolic_State::OCT_Symbolic_State(const std::vector<std::string> &loc_names, 
		   const Valuations &dvars, const PPL::NNC_Polyhedron &pol) 
            : Symbolic_State(loc_names, dvars, pol)
{
    oct_cvx = PPL::Octagonal_Shape<int>(cvx, PPL::ANY_COMPLEXITY);
    invariant_oct_cvx = get_invariant_oct();
    //invariant_oct_cvx = PPL::Octagonal_Shape<int>(invariant_cvx, PPL::ANY_COMPLEXITY);
    //accurate_cvx_is_inside = false;
    cvx.remove_higher_space_dimensions(0);
    invariant_cvx.remove_higher_space_dimensions(0);
}

//OCT_Symbolic_State::OCT_Symbolic_State(const OCT_Symbolic_State &ss)
//{
//    signature = ss.signature;
//    locations = ss.locations;
//    dvars = ss.dvars;
//    //valid = ss.valid;
//    //prior = ss.prior;
//    //decendants = ss.decendants;
//
//    oct_cvx = ss.oct_cvx;
//    invariant_oct_cvx = ss.oct_cvx;
//    //invariant_oct_cvx = ss.invariant_oct_cvx;
//    //accurate_cvx_is_inside = false;    
//    cvx.remove_higher_space_dimensions(0);
//    invariant_cvx.remove_higher_space_dimensions(0);
//}

shared_ptr<Symbolic_State> OCT_Symbolic_State::clone() const
{
    return make_shared<OCT_Symbolic_State> (*this);
}

bool OCT_Symbolic_State::contains(const std::shared_ptr<Symbolic_State> &pss) const
{
    if (not (signature == pss->get_signature()) ) return false;
    auto myptr = dynamic_pointer_cast<OCT_Symbolic_State>(pss); 
    return oct_cvx.contains(myptr->oct_cvx);
}

void OCT_Symbolic_State::discrete_step(const Combined_edge &edges)
{
    cout << "enter discrete step" << endl;
    //cout << "inside oct discrete step\n";
    VariableList cvars = MODEL.get_cvars();
    for ( auto &x : cvars)
        cout << x << " " << endl;

    PPL::Octagonal_Shape<int> guard_oct_cvx(cvars.size());
    /** 
     * The cvx obtained through updates in an
     * edge can have dimension higher than cvars.size().
     * */
    PPL::Octagonal_Shape<int> ass_oct_cvx(cvars.size()*2);

    Variables_Set vs;

    for (auto &e : edges.get_edges()) {
	    // find location source location with that name
	    bool found = false;
	    for (auto &p : locations) { 
	        if (p == &e.get_src_location()) {
		        found = true;
		        p = &e.get_dst_location();
	        }
        }
    	if (!found) throw string("ERROR!!! Cannot find src location ") +
	    		e.get_src_location().get_name();
	    // ss.loc_names[e.get_automaton_index()] = e.get_dest();
	    guard_oct_cvx.add_constraints(e.guard_to_Linear_Constraint(cvars, dvars));
	    Variables_Set vs2 = e.get_assignment_vars(cvars);
	    vs.insert(vs2.begin(), vs2.end());
	    // for (auto &a : e.assignments)
	    //     vs.insert(get_variable(a.get_var(), cvars));
	    ass_oct_cvx.add_constraints(e.ass_to_Linear_Constraint(cvars, dvars));
    }

    oct_cvx.intersection_assign(guard_oct_cvx);

    // constraints() function here costs more time than add_space_dimensions_and_embed
    /////ass_oct_cvx.add_constraints(oct_cvx.constraints());
    PPL::Octagonal_Shape<int> tmp = oct_cvx;
    tmp.add_space_dimensions_and_embed(cvars.size());
    ass_oct_cvx.intersection_assign(tmp);

    //cvx.add_space_dimensions_and_embed(cvars.size());
    //ass_cvx.intersection_assign(cvx);
    //cvx.remove_higher_space_dimensions(cvars.size());
    /** 
     * Before intersecting ass_cvx and cvx, we must remove 
     * the lower cvars.size() dimensions.
     * */
    PPL::Variables_Set lower_dims;
    for ( unsigned i = 0; i < cvars.size(); i++)
      lower_dims.insert(PPL::Variable(i));
    ass_oct_cvx.remove_space_dimensions(lower_dims);

    oct_cvx.unconstrain(vs);
    oct_cvx.intersection_assign(ass_oct_cvx);
    //PPL::Octagonal_Shape<int> invariant_oct_cvx = get_invariant_oct();
    invariant_oct_cvx = get_invariant_oct();
    oct_cvx.intersection_assign(invariant_oct_cvx);
    cout << "leave discrete step" << endl;
}

void OCT_Symbolic_State::continuous_step()
{
    cout << "enter continuous step" << endl;
    VariableList cvars = MODEL.get_cvars();
    //PPL::Octagonal_Shape<int> i_cvx(cvars.size());
#ifndef FORTS_PPL_PATCH
    PPL::Octagonal_Shape<int> r_cvx(cvars.size());    
    VariableList lvars = cvars;
#else
    vector<int> v_rates;
    for ( int i = 0; i < cvars.size(); i++)
        v_rates.push_back(1);
#endif

    for (auto p: locations) {
	    Linear_Constraint lc;
	    //i_cvx.add_constraints(p->invariant_to_Linear_Constraint(cvars, dvars));
#ifndef FORTS_PPL_PATCH
	    r_cvx.add_constraints(p->rates_to_Linear_Constraint(cvars, dvars, lvars));
#else
        p->abstract_rates(cvars, dvars, v_rates);
#endif
    }

#ifndef FORTS_PPL_PATCH
    for (auto &v : lvars) {
	    PPL::Variable var = get_ppl_variable(cvars, v);
	    Linear_Expr le;
	    le += 1;
	    AT_Constraint atc = (var == le);
	    r_cvx.add_constraint(atc);
    }
    oct_cvx.time_elapse_assign(r_cvx);
#else
    time_elapse_assign<int>(oct_cvx, v_rates);
#endif
    //oct_cvx.intersection_assign(i_cvx);
    oct_cvx.intersection_assign(invariant_oct_cvx);
    invariant_oct_cvx.remove_higher_space_dimensions(0);
    cout << "leave continuous step" << endl;
}

void OCT_Symbolic_State::print() const
{
    Symbolic_State::print();
    cout << "OCT : " << endl;
    cout << oct_cvx << endl;
    //cout << "Invariant OCT : " << endl;
    //cout << invariant_oct_cvx << endl;
}

bool OCT_Symbolic_State::is_empty() const
{
    return oct_cvx.is_empty();
}

bool OCT_Symbolic_State::is_bad()
{
    bool bad = Symbolic_State::is_bad();
    return bad;
//    if ( not bad) return bad;
//
//    list<shared_ptr<Symbolic_State> > chain = backtrack();
//
//    cout << "A possible bad chain ..." << endl;
//    for ( auto x : chain)
//        cout << x->get_loc_names() << " => ";
//    cout << endl << endl;;
//
//    for ( auto x : chain) {
//
//
//        auto myptr = dynamic_pointer_cast<OCT_Symbolic_State>(x); 
//        if ( not myptr->accurate_cvx_is_inside) {
//            cout << x->get_loc_names() << " => ";
//            myptr->build_accurate_cvx();
//            myptr->accurate_cvx_is_inside = true;
//        }
//
//        if (myptr->cvx.is_empty()) {
//            myptr->invalidate(); 
//            this->valid = false;
//            //break;
//            cout << endl;
//            return false;
//        }
//        //myptr->print();
//    }
//    cout << endl;
//    if ( is_valid())
//        return true;
//    else
//        return false;
}

//void OCT_Symbolic_State::build_accurate_cvx()
//{
//    if (prior == nullptr) {
//        cout << "cvx : " << cvx << endl;
//        cout << "invariant cvx : " << invariant_cvx << endl;
//        Symbolic_State::continuous_step();
//        accurate_cvx_is_inside = true;
//        return;
//    }
//
//    shared_ptr<Symbolic_State> p = make_shared<Symbolic_State>(*prior);
//
//    Combined_edge e = find_combined_edge(p, clone());
//    (*p).discrete_step(e);
//    (*p).continuous_step();
//    cvx = p->get_cvx();
//    invariant_cvx = p->get_invariant_cvx();
//    accurate_cvx_is_inside = true;
//}

const PPL::Octagonal_Shape<int>& OCT_Symbolic_State::get_oct() const
{
    return oct_cvx;
}

PPL::Octagonal_Shape<int> OCT_Symbolic_State::get_invariant_oct()
{
    VariableList cvars = MODEL.get_cvars();
    PPL::Octagonal_Shape<int> i_oct(cvars.size());
    //for ( auto it = automata.begin(); it != automata.end(); it++){
    for (auto p : locations) {
	Linear_Constraint lc;
	// string ln = ss.loc_names[it-automata.begin()];
	// Location &l = it->get_location_by_name(ln);

	i_oct.add_constraints(p->invariant_to_Linear_Constraint(cvars, dvars));
    }
    return i_oct;
}

int OCT_Symbolic_State::total_memory_in_bytes() const
{
    return oct_cvx.total_memory_in_bytes() + invariant_oct_cvx.total_memory_in_bytes();
}

bool OCT_Symbolic_State::equals(const std::shared_ptr<Symbolic_State> &pss) const
{
    bool res = Symbolic_State::equals(pss);
    if ( not res)
        return res;

    auto mypss = dynamic_pointer_cast<OCT_Symbolic_State>(pss); 

    return oct_cvx.contains(mypss->oct_cvx) && mypss->oct_cvx.contains(oct_cvx) 
            && invariant_oct_cvx.contains(mypss->invariant_oct_cvx) && mypss->invariant_oct_cvx.contains(invariant_oct_cvx);

}
