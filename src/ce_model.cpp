
#include <fstream>
#include <sstream>
#include <string>
#include <utility>
#include <algorithm>

#include "model.hpp"
#include "common.hpp"
#include "param_sstate.hpp"

using namespace std;
using namespace Parma_Polyhedra_Library::IO_Operators;
PPL::Constraint Model::negate_cs(const PPL::Constraint& c1) const
{
    if (c1.is_equality()) { 
        Linear_Expression e;
        for (dimension_type i = c1.space_dimension(); i-- > 0; )
            e += c1.coefficient(Variable(i)) * Variable(i);
        e += c1.inhomogeneous_term();
        Constraint c2 =  (e < 0); 
        return c2;
    }
    else {
        Linear_Expression e;
        for (dimension_type i = c1.space_dimension(); i-- > 0; )
            e += c1.coefficient(Variable(i)) * Variable(i);
        e += c1.inhomogeneous_term();
        Constraint c2 = c1.is_strict_inequality() ? (e <= 0) : (e < 0); 
        return c2;
    }
}

void Model::CE()
{

    auto init = init_param_sstate();
    //PPL::NNC_Polyhedron cvx = init->get_cvx();

    //while (true) {

        list<shared_ptr<Symbolic_State> > next;
        list<shared_ptr<Symbolic_State> > current;
        Space.clear();
        //init->refine_cvx(param_region);
        current.push_back(init);
        int step = 0; 
        bool complete = false;
        while (true) {
            for( auto it = current.begin(); it != current.end(); it++) {
                if(not (*it)->is_valid())
                    continue;
                vector<shared_ptr<Symbolic_State> > nsstates = Post(*it);
                for( auto iit = nsstates.begin(); iit != nsstates.end(); iit++) {
                    //(*iit)->mark_prior(*it);
                    if( (*iit)->is_empty())
                        continue;
                    // Even if *iit is not empty, it may become empty by taking counter-examples into account
                    bool false_reach = false;
                    for ( auto &x : ces) {
                        PPL::NNC_Polyhedron poly((*iit)->get_cvx());
                        poly.intersection_assign(x);
                        if( poly.is_empty()) {
                            false_reach = true;
                            break;
                        }
                    }
                    if( false_reach) continue;

                if ( (*iit)->is_bad()) {
                    PPL::NNC_Polyhedron c = (*iit)->get_cvx();
                    unconstrain_to_parameters(c);
                    //map_to_parameters(c);
                    if( not contained_in(c, ces)) {
                        remove_contained_elements(c, ces);
                        ces.push_back(c);
                        refine_with_counter_example(c, Space);
                        refine_with_counter_example(c, current);
                        refine_with_counter_example(c, next);
                    }

                    if(not (*it)->is_valid())
                        break;
                    continue;
                }
                if( contained_in(*iit, current))
                    continue;
                if( contained_in(*iit, next))
                    continue;
                if( contained_in(*iit, Space))
                    continue;
                remove_included_sstates_in_a_list(*iit, next);
                next.push_back(*iit);
            }
         }
        cout << "Step : " << step++ << endl;
        cout << "Space : " << Space.size() << ", ";
        cout << "current : " << current.size() << ", ";
        cout << "next : " << next.size() << endl;
        current.splice(Space.begin(), current);
        if( next.size() == 0) {
            complete = true;
            break;
        }
        if ( step >= bound)
            break;
	    current.splice(current.begin(), next);
    }

    if( not complete) {
        for ( auto & x : next) {
            if ( not x->is_valid())
                continue;
            auto y = x->get_prior();
            if ( not y->is_valid())
                continue;
            PPL::NNC_Polyhedron poly_x = x->get_cvx();
            PPL::NNC_Polyhedron poly_y = y->get_cvx();
            map_to_parameters(poly_x);
            map_to_parameters(poly_y);
            //PPL::Constraint cs_x (PPL::Variable(0)<PPL::Variable(0)); // = *(poly_x.constraints().begin());
            PPL::Constraint cs_x = *(poly_x.constraints().begin());
            cs_x = negate_cs(cs_x);
    //        bool cs_x_found = false;
    //        for ( auto & cs : poly_x.constraints()) {
    //            //PPL::NNC_Polyhedron nu(cs);
    //            Constraint cs2 = negate_cs(cs);
    //            PPL::NNC_Polyhedron tmp1 = param_region;
    //            tmp1.add_constraint(cs2);
    //            if ( tmp1.is_empty())
    //                continue;
    //            if(not cs_x_found ) {
    //                cs_x = cs2;
    //                cs_x_found = true;
    //            }
    //            PPL::NNC_Polyhedron tmp2 = poly_y;
    //            tmp2.add_constraint(cs);
    //            if ( tmp2.is_empty()) {
    //                cs_x = cs2;
    //                break;
    //            }
    //        }

    //        if( not cs_x_found) {
    //            cout << "cs x not found : " << endl << poly_x << endl;
    //            x->print();
    //            cout << "backtrack " << endl;
    //            auto xt = x->get_prior();
    //            while( xt != nullptr) {
    //                xt->print();
    //                NNC_Polyhedron tmp = x->get_cvx();
    //                map_to_parameters(tmp);
    //                cout << tmp << endl;
    //                xt = xt->get_prior();
    //                //cout << "poly_y : " << endl << poly_y << endl;
    //            }
    //        }
            PPL::NNC_Polyhedron tmp3(parameters.size());
            tmp3.add_constraint(cs_x);
            ending_points.push_back(tmp3);
        }
//        }
    }

    cout << "Counter examples : " << endl;
    for ( auto & x: ces) {
        cout << x << endl;
    }
    cout << "Ending points : " << endl;
    for ( auto & x: ending_points) {
        cout << x << endl;
    }

    cout << "size of next : " << next.size() << endl;
}

shared_ptr<Symbolic_State> Model::init_param_sstate()
{
    vector<std::string> loc_names;

    // The constructor of Symbolic_State needs the initial name of each automaton
    for (auto it = automata.begin(); it != automata.end(); it++) {
	    loc_names.push_back(it->get_init_location());
    }

    // parameters are regarded as continuous variables with rate 0
    for( auto &x : parameters) 
        cvars.insert(x.name);

    // Never forget to check if the model is consistent
    check_consistency();

    PPL::NNC_Polyhedron cvx(cvars.size());
    cvx.add_constraints(init_constraint.to_Linear_Constraint(cvars, dvars));
    
    // To build the parameter space we're going to explore.
    param_region = cvx;
    Variables_Set vars;
    int index = 0;
    for( auto x : cvars) {
        if( not is_parameter(x))
            vars.insert(PPL::Variable(index));
        index ++;
    }
    // Note that "param_region" also includes non-parameter continuous variables
    param_region.unconstrain(vars);


    shared_ptr<Symbolic_State> init = make_shared<Param_Symbolic_State>(loc_names, dvars, cvx);

    // Let's have a continuous step ;)
    init->continuous_step();
    return init;
}    

bool Model::contained_in(const PPL::NNC_Polyhedron &c, const std::list<NNC_Polyhedron> &lcs)
{
    for (auto &x : lcs)
        if(x.contains(c))
            return true;
    return false;
}

int Model::remove_contained_elements(const PPL::NNC_Polyhedron &c, std::list<NNC_Polyhedron> &lcs)
{
    int res = 0;
    auto it = lcs.begin();
    while (it != lcs.end()) {
        if ( c.contains(*it)) {
            res ++;
            it = lcs.erase(it);
        }
        else it ++;
    }
    return res;
}

void Model::set_bound(int b) { bound = b;}

void Model::unconstrain_to_parameters(PPL::NNC_Polyhedron &poly)
{
    PPL::Variables_Set vars;
    int pos = 0;
    for( auto &x:cvars) {
        //cout << x << endl;
        if(not is_parameter(x))
            vars.insert(PPL::Variable(pos));
        pos++;
    }
    poly.unconstrain(vars);
}

void Model::refine_with_counter_example(const PPL::NNC_Polyhedron& ce, std::list<std::shared_ptr<Symbolic_State> > &lss)
{
    for( auto&x: lss) {
        PPL::NNC_Polyhedron poly(x->get_cvx());
        poly.intersection_assign(ce);
        if( poly.is_empty()) {
           x->invalidate(); 
        }
    }
}
