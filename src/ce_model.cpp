
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

static PPL::Constraint negate_cs(PPL::Constraint c1)
{
    if (c1.is_equality()) { 
        //cout << c1 << endl;
        //throw string("Constraint c1 is not an inequality"); // << c1 << endl;
        Linear_Expression e;
        for (dimension_type i = c1.space_dimension(); i-- > 0; )
            e += c1.coefficient(Variable(i)) * Variable(i);
        e += c1.inhomogeneous_term();
        // This is incomplete and only for test, as we also need to consider the case such that e>0.
        // And we need also utilize the "param_region" to decide whether to return "e>0" or "e<0".
        Constraint c2 =  (e < 0); //cout << "Complement c2: " << c2 << endl;
        return c2;
    }
    else {
        Linear_Expression e;
        for (dimension_type i = c1.space_dimension(); i-- > 0; )
            e += c1.coefficient(Variable(i)) * Variable(i);
        e += c1.inhomogeneous_term();
        Constraint c2 = c1.is_strict_inequality() ? (e <= 0) : (e < 0); //cout << "Complement c2: " << c2 << endl;
        return c2;
    }
}

void Model::CE()
{

    auto init = init_param_sstate();
    PPL::NNC_Polyhedron cvx = init->get_cvx();

    Variables_Set vars;
    int index = 0;
    for( auto x : cvars) {
        if( not is_parameter(x))
            vars.insert(PPL::Variable(index));
        index ++;
    }
    cvx.unconstrain(vars);
    param_region = cvx;

    list<shared_ptr<Symbolic_State> > next;
    list<shared_ptr<Symbolic_State> > current;
    current.push_back(init);
    int step = 0; 
    bool complete = false;
    //bound = 20;
    while (true) {
        for( auto it = current.begin(); it != current.end(); it++) {
            vector<shared_ptr<Symbolic_State> > nsstates = Post(*it);
            for( auto iit = nsstates.begin(); iit != nsstates.end(); iit++) {
                (*iit)->mark_prior(*it);
                if( (*iit)->is_empty())
                    continue;
                if ( (*iit)->is_bad()) {
                    PPL::NNC_Polyhedron c = (*iit)->get_cvx();
                    map_to_parameters(c);
                    if( not contained_in(c, ces)) {
                        remove_contained_elements(c, ces);
                        ces.push_back(c);
                    }
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
            auto y = x->get_prior();
            PPL::NNC_Polyhedron poly_x = x->get_cvx();
            PPL::NNC_Polyhedron poly_y = y->get_cvx();
            map_to_parameters(poly_x);
            map_to_parameters(poly_y);
            //PPL::Constraint cs_x (PPL::Variable(0)<PPL::Variable(0)); // = *(poly_x.constraints().begin());
            PPL::Constraint cs_x = *(poly_x.constraints().begin());
            cs_x = negate_cs(cs_x);
            bool cs_x_found = false;
            for ( auto & cs : poly_x.constraints()) {
                //PPL::NNC_Polyhedron nu(cs);
                Constraint cs2 = negate_cs(cs);
                PPL::NNC_Polyhedron tmp1 = param_region;
                tmp1.add_constraint(cs2);
                if ( tmp1.is_empty())
                    continue;
                if(not cs_x_found ) {
                    cs_x = cs2;
                    cs_x_found = true;
                }
                PPL::NNC_Polyhedron tmp2 = poly_y;
                tmp2.add_constraint(cs);
                if ( tmp2.is_empty()) {
                    cs_x = cs2;
                    break;
                }
            }

            if( not cs_x_found) {
                cout << "cs x not found : " << endl << poly_x << endl;
                x->print();
                cout << "backtrack " << endl;
                auto xt = x->get_prior();
                while( xt != nullptr) {
                    xt->print();
                    NNC_Polyhedron tmp = x->get_cvx();
                    map_to_parameters(tmp);
                    cout << tmp << endl;
                    xt = xt->get_prior();
                    //cout << "poly_y : " << endl << poly_y << endl;
                }
            }
            PPL::NNC_Polyhedron tmp3(param_region.space_dimension());
            tmp3.add_constraint(cs_x);
            ending_points.push_back(tmp3);
        }
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
    vector<Location *> locs;
    vector<std::string> loc_names;
    PPL::NNC_Polyhedron cvx(cvars.size());

    string ln="";
    for (auto it = automata.begin(); it != automata.end(); it++) {
	    loc_names.push_back(it->get_init_location());
	    locs.push_back(&(it->get_location_by_name(it->get_init_location())));
	    ln += it->get_init_location();
    }

    for( auto &x : parameters) 
        cvars.insert(x.name);

    check_consistency();

    PPL::NNC_Polyhedron dual_cvx(cvars.size());
    dual_cvx = NNC_Polyhedron(init_constraint.to_Linear_Constraint(cvars, dvars));

    shared_ptr<Symbolic_State> init;

    init = make_shared<Param_Symbolic_State>(loc_names, dvars, dual_cvx);

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
