#include <fstream>
#include <sstream>
#include <string>
#include <utility>
#include <algorithm>

#include "model.hpp"
#include "common.hpp"
#include "dual_sstate.hpp"

using namespace std;
using namespace Parma_Polyhedra_Library::IO_Operators;

PPL::Constraint Model::negate_cs(const PPL::Constraint& c1, const Valuations& ref) const
{
    if (c1.is_equality()) { 
        Linear_Expression e;
        for (dimension_type i = c1.space_dimension(); i-- > 0; )
            e += c1.coefficient(Variable(i)) * Variable(i);
        e += c1.inhomogeneous_term();
        Constraint c2 =  (e < 0); 
        if( not incompatible(c2, ref))
            return c2;
        Constraint c3 =  (e > 0); 
        return c3;
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


bool Model::incompatible(const PPL::Constraint &cs, const Valuations &pi0) const
{
    PPL::NNC_Polyhedron poly(pi0.size());
    poly.add_constraint(cs);

    PPL::NNC_Polyhedron poly0(pi0.size());
    int pos = 0;
    for( auto &x : pi0) {
        poly0.add_constraint(PPL::Variable(pos++)==x.second);
    }

    return not poly.contains(poly0);
}

void Model::efficient_BEEP(const Valuations &pi0)
{
    /** Keep a copy of real discrete variables. **/
    Valuations old = dvars;
    dvars.insert(pi0.begin(), pi0.end());


    shared_ptr<Symbolic_State> init = beep_init_sstate();

    list<shared_ptr<Symbolic_State> > next;
    list<shared_ptr<Symbolic_State> > current;
    current.push_back(init);
    
    /** Constraint on parameters obtained from acyclic trace and reachable part of an unreachable trace. */
    PPL::NNC_Polyhedron conjunction_part(parameters.size());

    /** States that identify cyclic traces */
    vector<shared_ptr<Symbolic_State> > dominating;
    vector<shared_ptr<Symbolic_State> > dominated;

    int step = 0;

    while( true) {
        for (auto it = current.begin(); it != current.end(); it++) {
            vector<shared_ptr<Symbolic_State> > nsstates = Post(*it);
            for ( auto &x : nsstates) {
                x->mark_prior(*it);
                if (x->is_empty()) { // an unreachable trace is met
                    auto myx = dynamic_pointer_cast<Dual_Symbolic_State>(x);
                    PPL::NNC_Polyhedron unreach = myx->get_dual_cvx();

                    if( not unreach.is_empty()) {
                        map_to_parameters(unreach, myx->get_dual_cvars());
                        for ( auto & cs : unreach.constraints()) {
                            if(incompatible(cs, pi0)) {
                                conjunction_part.add_constraint(negate_cs(cs, pi0));
                                break;
                            }
                        }
                    }
                    continue;
                }
                if(x->is_bad()) { // The bad location is met and a bad tile can be returned directly
                    auto myx = dynamic_pointer_cast<Dual_Symbolic_State>(x); 
                    PPL::NNC_Polyhedron bad_cvx = myx->get_dual_cvx();
                    map_to_parameters(bad_cvx, myx->get_dual_cvars());
                    bad_tiles.push_back(bad_cvx);
                    cout << "A bad tile : " << bad_cvx << endl;
                    dvars = old;
                    return;
                }


                /** To detect a cycle, we backtrack and check if some precedent state contains current state. */
                if( dominated_relation(x, current, dominating, dominated) +
                    dominated_relation(x, next, dominating, dominated) +
                    dominated_relation(x, Space, dominating, dominated) > 0) 
                    continue;


                //if(x->no_outgoings()) { // acyclic trace is here
                //    auto myx = dynamic_pointer_cast<Dual_Symbolic_State>(x);
                //    PPL::NNC_Polyhedron reach = myx->get_dual_cvx();
                //    map_to_parameters(reach, myx->get_dual_cvars());
                //    conjunction_part.intersection_assign(reach);
                //}

                next.push_back(x);

            }
        }
        Space.splice(Space.end(), current);
        if( next.size() == 0) {
            break;
        }
        current.splice(current.end(), next);

    }

    cout << "---------------------------------------------------" << endl;
    cout << "Number of states    : " << Space.size() << endl;
    cout << "Domination  : " << dominated.size() << endl;
    cout << "---------------------------------------------------" << endl;
    PPL::NNC_Polyhedron good_tile(conjunction_part);

    //cout << "good tile before growing parameter constraints " << good_tile << endl;

    for( int i = 0; i < dominating.size(); i++) {
        auto mying = dynamic_pointer_cast<Dual_Symbolic_State>(dominating[i]); 
        auto myed = dynamic_pointer_cast<Dual_Symbolic_State>(dominated[i]); 
        grow_by_steps(good_tile, dominating[i], dominated[i], pi0);
    }

    good_tile.intersection_assign(param_region);
    if(not good_tile.is_empty())
        efficient_good_tiles.push_back(good_tile);
    cout << "A good tile : " << good_tile << endl;
    dvars = old;
}


void Model::grow_by_steps(NNC_Polyhedron &good_tile, const shared_ptr<Symbolic_State> &dominating, const shared_ptr<Symbolic_State> &dominated, const Valuations &pi0, const double step) const
{
    auto mydominating = dynamic_pointer_cast<Dual_Symbolic_State>(dominating);
    auto mydominated = dynamic_pointer_cast<Dual_Symbolic_State>(dominated);

    const NNC_Polyhedron & dominating_cvx = mydominating->get_dual_cvx();
    const NNC_Polyhedron & dominated_cvx = mydominated->get_dual_cvx();

    // A copy of original cvx we are going to manupilate
    NNC_Polyhedron ing(dominating_cvx);
    NNC_Polyhedron ed(dominated_cvx);
    ing.concatenate_assign(good_tile);
    ed.concatenate_assign(good_tile);

    auto dual_cvars = mydominating->get_dual_cvars();

    /********************************************************************************************************************************************/
    /** If ing contains ed for all parameters valuations inside current good tile, then we are lucky and there is no need to change good_tile **/
    int j = 0;
    for( auto &x: dual_cvars) {
        if( is_parameter(x)) {
            ing.add_constraint(Variable(j) == Variable(dual_cvars.size()+j));
            ed.add_constraint(Variable(j) == Variable(dual_cvars.size()+j));
            j++;
        }
    }

    if( ing.contains(ed))
        return;
    /********************************************************************************************************************************************/
    // We are going to grow bounds on each parameter
    vector<double> bounds;
    for( auto &x : pi0)
        bounds.push_back(x.second);

    NNC_Polyhedron refined_tile = good_tile;

    for( int i = 0; i < bounds.size(); i++) {

        //cout << "parameter " << valuation_index_to_parameter(pi0,i).name << " " << bounds[i] << endl;
        double i_upper_bound = bounds[i], i_lower_bound = bounds[i]; 
        double i_trial_upper_bound = bounds[i], i_trial_lower_bound = bounds[i]; 
        // We first increase the i-th parameter in positive direction, then negative direction
        bool positive_dir = true;
        // "true" denotes "<=" or ">="; and "false" denotes "<" or ">" 
        bool up_cut_flag = true, down_cut_flag = true;
        // if the past value of parameter is included in the tile, if two successive values are both outside the tile, there is no need to go further
        bool past = true;
        // currently refined tile
        NNC_Polyhedron curr_refined_tile = refined_tile;
        cut_tile(i, i_trial_upper_bound, up_cut_flag, i_lower_bound, down_cut_flag, refined_tile);

        /** Start to compute the region of the i-th parameter. */
        while( true) {
            /** Start to increase the i-th parameter step by step. */
            if( positive_dir) {
                //cout << "positive direction" << endl;
                bool up = not up_cut_flag;
                bool down = down_cut_flag;
                if( up == false) { // open cut "<"
                    i_trial_upper_bound += step;
                    //cout << "i_trial_upper_bound = " << i_trial_upper_bound << endl;
                }

                if( be_outside(i, i_trial_upper_bound, refined_tile)) {
                    if( past) {
                        past = false;
                    } 
                    else {
                        positive_dir = false;
                        past = true;
                        continue;
                    }
                }
                //cout << "should not be outside " << endl;

                NNC_Polyhedron trial_refined_tile = refined_tile;
                cut_tile(i, i_trial_upper_bound, up, i_lower_bound, down, trial_refined_tile);

                ing = NNC_Polyhedron(dominating_cvx);
                ed = NNC_Polyhedron(dominated_cvx);

                ing.concatenate_assign(trial_refined_tile);
                ed.concatenate_assign(trial_refined_tile);

                int j = 0;
                for( auto &x: dual_cvars) {
                    if( is_parameter(x)) {
                        ing.add_constraint(Variable(j) == Variable(dual_cvars.size()+j));
                        ed.add_constraint(Variable(j) == Variable(dual_cvars.size()+j));
                        j++;
                    }
                }

                if( ing.contains(ed)) {
                    i_upper_bound = i_trial_upper_bound;
                    up_cut_flag = up;
                    curr_refined_tile = trial_refined_tile;
                }
                else {
                //cout << "should not be contain " << endl;
                    positive_dir = false;
                    past = true;
                    //curr_refined_tile = trial_refined_tile;
                    continue;
                }

            }
            else {
                //cout << "now turn to check the negative direction" << endl;
                bool down = not down_cut_flag;
                bool up = up_cut_flag;
                if( down == false) { // open cut ">"
                    i_trial_lower_bound -= step;
                }
                //cout << "i_trial_lower_bound = " << i_trial_lower_bound << endl;

                if( be_outside(i, i_trial_lower_bound, refined_tile)) {
                    if( past) {
                        past = false;
                    } 
                    else {
                        break;
                    }
                }

                NNC_Polyhedron trial_refined_tile = refined_tile;
                cut_tile(i, i_upper_bound, up, i_trial_lower_bound, down, trial_refined_tile);

                ing = NNC_Polyhedron(dominating_cvx);
                ed = NNC_Polyhedron(dominated_cvx);

                ing.concatenate_assign(trial_refined_tile);
                ed.concatenate_assign(trial_refined_tile);

                int j = 0;
                for( auto &x: dual_cvars) {
                    if( is_parameter(x)) {
                        ing.add_constraint(Variable(j) == Variable(dual_cvars.size()+j));
                        ed.add_constraint(Variable(j) == Variable(dual_cvars.size()+j));
                        j++;
                    }
                }

                if( ing.contains(ed)) {
                    i_lower_bound = i_trial_lower_bound;
                    down_cut_flag = down;
                    curr_refined_tile = trial_refined_tile;
                }
                else {
                    break;
                }

            }


        }

        refined_tile = curr_refined_tile;
    }
    good_tile = refined_tile;
}

void Model::cut_tile(const int i, const int i_upper_bound, const bool up, const int i_lower_bound, const bool down, PPL::NNC_Polyhedron &trial_refined_tile) const
{
    PPL::Constraint l, u;
    if( up) {
        u = (Variable(i)<=i_upper_bound);
    }
    else
        u = (Variable(i)<i_upper_bound);

    if( down) {
        l = (Variable(i)>=i_lower_bound);
    }
    else
        l = (Variable(i)>i_lower_bound);
    
    trial_refined_tile.add_constraint(l);
    trial_refined_tile.add_constraint(u);
}

bool Model::be_outside(const int i, const double v, const PPL::NNC_Polyhedron &domain) const
{
    auto p = parameters[i];
    if( v < p.min or v > p.max)
        return true;

    NNC_Polyhedron tmp (domain);
    tmp.add_constraint(Variable(i) == v);
    if( tmp.is_empty())
        return true;
    else
        return false;
}

struct efficient_points_com_class {
    bool operator() (const vector<double> &p1, const vector<double> &p2) {
        if (p1.size() != p2.size())
            throw("Uncomparable points ...");
        for( int i = 0; i < p1.size(); i++) {
            if( p1[i] < p2[i]) return true;
            if( p2[i] < p1[i]) return true;
        }
        return true;
    }
} efficient_points_com;

void Model::efficient_print_points(string fname) const
{
    vector<Parameter> parameters1(parameters);
    sort(parameters1.begin(), parameters1.end());
    string X = parameters1[0].name;
    string Y = parameters1[1].name;
    string graph_command = string("graph") + string(" -T ps -C -X ") + X + string(" -Y ") + Y + string(" ");

    int index = 0;
    for( auto &x : efficient_good_tiles) {
        //for (auto it = x.begin(); it != x.end(); it++) {
        {
            const PPL::NNC_Polyhedron &cp = x; //it->pointset(); 
            PPL::C_Polyhedron cpp(cp);
            const PPL::Generator_System &gs = cpp.generators();
            cout << cpp << endl;

            ofstream ofs;
            string output = fname + string("-good-tile-") + to_string(index++);
            ofs.open(output.c_str());
            vector< vector<double> > points;
            for( auto jt = gs.begin(); jt != gs.end(); jt ++) {
                if( !jt->is_point()) {
                }
                vector<double> point;
                for( auto dim = jt->space_dimension(); dim -- > 0;) {
                    stringstream ss1 (stringstream::in | stringstream::out);
                    stringstream ss2 (stringstream::in | stringstream::out);

                    ss1 << jt->coefficient(PPL::Variable(dim));
                    ss2 << jt->divisor();
                    double coe, div;
                    ss1 >> coe; ss2 >> div;
                    point.push_back(coe/div);
                    //cout << coe/div << endl << endl;
                }
                points.push_back(point);
            }
            sort(points.begin(), points.end(), efficient_points_com);
            for( auto &x: points) {
                //for( int i = 0; i < x.size(); i++) {
                for( int i = x.size()-1; i >= 0; i--) {
                    //if( i != 0) ofs << " ";
                    if( i != x.size()-1) ofs << " ";
                    ofs << x[i];
                }
                
                ofs << endl;
            }
            // to draw a region, we need to output the first point in the end also
            auto xt = points.begin();
            for( int i = xt->size()-1; i >= 0; i--) {
                if( i != xt->size()-1) ofs << " ";
                ofs << (*xt)[i];
            }
            ofs << endl;

            graph_command += string(" -m 2 -q 1 ")  + output + string(" ");

        }
    }
    
    // to print bad tiles
    index = 0;
    for( auto &x : bad_tiles) {

        PPL::C_Polyhedron cpp(x);
        const PPL::Generator_System &gs = cpp.generators();
        //cout << "cpp : " << cpp << endl;

        //const PPL::Generator_System &gs = x.generators();

        ofstream ofs;
        string output = fname + string("-bad-tile-") + to_string(index++);
        ofs.open(output.c_str());

        vector< vector<double> > points;
        for( auto jt = gs.begin(); jt != gs.end(); jt ++) {
            //cout << *jt << endl;
            //if( !jt->is_point()) //throw ("Not a point here ...");
            //    cout << jt->type() << endl;
            vector<double> point;
            for( auto dim = jt->space_dimension(); dim -- > 0;) {
                stringstream ss1 (stringstream::in | stringstream::out);
                stringstream ss2 (stringstream::in | stringstream::out);

                ss1 << jt->coefficient(PPL::Variable(dim));
                ss2 << jt->divisor();
                double coe, div;
                ss1 >> coe; ss2 >> div;
                point.push_back(coe/div);
            }
            points.push_back(point);
        }
        sort(points.begin(), points.end(), efficient_points_com);
        for( auto &x: points) {
            //for( int i = 0; i < x.size(); i++) {
            //    if( i != 0) ofs << " ";
            for( int i = x.size()-1; i >= 0; i--) {
                if( i != x.size()-1) ofs << " ";
                ofs << x[i];
            }
            ofs << endl;
        }
        // to draw a region, we need to output the first point in the end also
        auto xt = points.begin();
        for( int i = xt->size()-1; i >= 0; i--) {
            if( i != xt->size()-1) ofs << " ";
            ofs << (*xt)[i];
        }
        ofs << endl;
        graph_command += string(" -m 1 -q 1 ")  + output + string(" ");
    }

    string graph_output = fname + string(".ps");
    graph_command += string(" -L \"\" > ") + graph_output;
    string graph_command_fname = fname+string("-graph.sh");
    ofstream graph_ofs;
    graph_ofs.open(graph_command_fname.c_str());
    graph_ofs << graph_command;
}

bool Model::efficient_in_a_tile(const Valuations& v) const
{
    PPL::NNC_Polyhedron point_poly(parameters.size());
    int pos = 0;
    for ( auto it = v.begin(); it != v.end(); it++) {
        point_poly.add_constraint(PPL::Variable(pos) == it->second);
        pos++;
    }

    for( auto &x : efficient_good_tiles) {
        if(x.contains(point_poly)) {
            return true;
        }
    }
    for( auto &x : bad_tiles) {
        if(x.contains(point_poly)) {
            return true;
        }
    }

    return false;
}

void Model::efficient_BEEP()
{
    cout << "We are inside the new beep" << endl;
    /////////////////// This part is only used for call check_consistency() //////////////////////////////////
    Valuations param_vars;
    for (auto &x : parameters) {
        param_vars[x.name] = x.min;
    }
    Valuations old = dvars;
    dvars.insert(param_vars.begin(), param_vars.end());
    check_consistency();
    dvars = old;
    //////////////////////////////////////////////////////////////////////////////////////////////////////////
    cout << "We passed the consistency checking" << endl;

    vector<Valuations> points;
    vector<Valuations> passed_points;
    Valuations v;
    for( auto & x : parameters) {
        v.insert(pair<string,int>(x.name,x.min));
    }
    points.push_back(v);

    //for( auto &x: points){
    while(points.size() != 0) {
        auto x = points[0];
        UNReach.clear();
        Acyclic.clear();
        BS.clear();
        Contained.clear();
        Space.clear();

        cout << "Point : ";
        for( auto &y: x) {
            cout << y.first << "==" << y.second << " ";
        }
        if( efficient_in_a_tile(x)) {
            cout << "is in a generated tile." << endl;
            passed_points.push_back(points[0]);
            vector<Valuations> next_points = increase_by_one_step(x);
            points.erase(points.begin());

            for( auto &y: next_points) {
                if( (not existing_point(y, points)) and (not existing_point(y, passed_points)))
                    points.push_back(y);
            }
            continue;
        }
        cout << "is not contained in a generated tile." << endl;

        efficient_BEEP(x);

        passed_points.push_back(points[0]);
        vector<Valuations> next_points = increase_by_one_step(x);
        points.erase(points.begin());

        for( auto &y: next_points) {
            if( (not existing_point(y, points)) and (not existing_point(y, passed_points)))
                points.push_back(y);
        }
    }

    cout << endl << endl;
    cout << "Good tiles : " << endl;
    int index = 0;
    for( auto &x : efficient_good_tiles) {
        cout << "== good tile " << index++ << " ==" << endl;
        cout << x << endl;
    }
    index = 0;
    cout << "Bad tiles : " << endl;
    for( auto &x : bad_tiles) {
        cout << "== bad tile " << index++ << " ==" << endl;
        cout << x << endl;
    }
}

