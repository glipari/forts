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

/** Model with parameters. */

void Model::add_param(const Parameter &param)
{
    parameters.push_back(param);
}

const std::vector<Parameter>& Model::get_parameters() const
{
    return parameters;
}

static vector<Valuations> generate_valuations_list(const vector<Parameter> &vp)
{
    vector<Valuations> vv;
    if(vp.size() == 1) {
        for( int i = vp[0].min; i <= vp[0].max; i++) {
            Valuations v;
            v.insert(pair<string,int>(vp[0].name, i));
            vv.push_back(v);
        }
        return vv;
    }

    // else
    vector<Parameter> nvp = vp;
    nvp.pop_back();
    vv = generate_valuations_list(nvp);
    vector<Valuations> res_vv;
    for( auto &x: vv) {
        for( int i = vp.back().min; i <= vp.back().max; i++) {
            Valuations v = x;
            v.insert(pair<string,int>(vp.back().name, i));
            res_vv.push_back(v);
        }
    }
    return res_vv;
}

bool Model::is_parameter(const std::string& s) const {
    for( auto &x: parameters) {
        if( x.name == s)
            return true;
    }
    return false;
}

Parameter Model::get_parameter_by_name(const std::string& s) const {
    for( auto &x: parameters) {
        if( x.name == s)
            return x;
    }
    throw string("no such a parameter named \"") + s + string("\" ...\n");
}

void Model::map_to_parameters(PPL::NNC_Polyhedron &poly, const VariableList& cvars)
{
    PPL::Variables_Set vars;
    int pos = 0;
    for( auto &x:cvars) {
        //cout << x << endl;
        if(not is_parameter(x))
            vars.insert(PPL::Variable(pos));
        pos++;
    }
    poly.remove_space_dimensions(vars);
}

void Model::map_to_parameters(PPL::Pointset_Powerset<PPL::NNC_Polyhedron> &poly, const VariableList& cvars)
{
    PPL::Variables_Set vars;
    int pos = 0;
    for( auto &x:cvars) {
        //cout << x << endl;
        if(not is_parameter(x))
            vars.insert(PPL::Variable(pos));
        pos++;
    }
    poly.remove_space_dimensions(vars);
}


void Model::map_to_parameters(PPL::NNC_Polyhedron &poly)
{
    PPL::Variables_Set vars;
    int pos = 0;
    for( auto &x:cvars) {
        //cout << x << endl;
        if(not is_parameter(x))
            vars.insert(PPL::Variable(pos));
        pos++;
    }
    poly.remove_space_dimensions(vars);
}

bool Model::in_a_tile(const Valuations& v) const
{
    PPL::Pointset_Powerset<PPL::NNC_Polyhedron> point_pp(parameters.size());
    PPL::NNC_Polyhedron point_poly(parameters.size());
    int pos = 0;
    for ( auto it = v.begin(); it != v.end(); it++) {
        point_pp.add_constraint(PPL::Variable(pos) == it->second);
        point_poly.add_constraint(PPL::Variable(pos) == it->second);
        pos++;
    }

    for( auto &x : good_tiles) {
        if(x.contains(point_pp)) {
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

struct points_com_class {
    bool operator() (const vector<double> &p1, const vector<double> &p2) {
        if (p1.size() != p2.size())
            throw("Uncomparable points ...");
        for( int i = 0; i < p1.size(); i++) {
            if( p1[i] < p2[i]) return true;
            if( p2[i] < p1[i]) return true;
        }
        return true;
    }
} points_com;

void Model::print_points(string fname) const
{
    vector<Parameter> parameters1(parameters);
    sort(parameters1.begin(), parameters1.end());
    string X = parameters1[0].name;
    string Y = parameters1[1].name;
    string graph_command = string("graph") + string(" -T ps -C -X ") + X + string(" -Y ") + Y + string(" ");

    int index = 0;
    // to print good tiles
    for( auto &x : good_tiles) {
        for (auto it = x.begin(); it != x.end(); it++) {
            const PPL::NNC_Polyhedron &cp = it->pointset(); 
            //cout << "cp : " << cp <<endl;
            //const PPL::Generator_System &gs = cp.generators();
            //cout << "gs : " << gs << endl;
            PPL::C_Polyhedron cpp(cp);
            const PPL::Generator_System &gs = cpp.generators();

            ofstream ofs;
            string output = fname + string("-good-tile-") + to_string(index++);
            ofs.open(output.c_str());
            //cout << output << endl;
            vector< vector<double> > points;
            for( auto jt = gs.begin(); jt != gs.end(); jt ++) {
              //  cout << *jt << endl;
                if( !jt->is_point()) {
                    //cout << "Not a point here ..." << endl;
                //    cout << jt->type() << endl;
                    //continue;
                    //throw ("Not a point here ...");
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
            sort(points.begin(), points.end(), points_com);
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
        sort(points.begin(), points.end(), points_com);
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

bool Model::contained_in_then_store(const std::shared_ptr<Symbolic_State> &ss, const std::list<std::shared_ptr<Symbolic_State> > &lss, PPL::NNC_Polyhedron& conjunction_part)
{
    for ( auto it = lss.begin(); it != lss.end(); it++) {
	    bool f = (*it)->contains(ss);
	    if (f) {
            auto myx1 = dynamic_pointer_cast<Dual_Symbolic_State>(ss); 
            auto myx2 = dynamic_pointer_cast<Dual_Symbolic_State>(*it); 
            NNC_Polyhedron cvx1 = myx1->get_dual_cvx();
            NNC_Polyhedron cvx2 = myx2->get_dual_cvx();

            auto dual_cvars = myx1->get_dual_cvars();

            cvx1.concatenate_assign(cvx2);
            PPL::Variables_Set vars;
            int pos = 0;
            for( auto &x : dual_cvars){
                cvx1.add_constraint(PPL::Variable(pos) == PPL::Variable(pos+dual_cvars.size()));
                if( not is_parameter(x)) {
                    vars.insert(PPL::Variable(pos));
                    vars.insert(PPL::Variable(pos+dual_cvars.size()));
                }
                else
                    vars.insert(PPL::Variable(pos+dual_cvars.size()));
                pos ++;
            }
            cvx1.remove_space_dimensions(vars);

            conjunction_part.intersection_assign(cvx1);
            return true;
        }
    }
    return false;

}

shared_ptr<Symbolic_State> Model::beep_init_sstate()
{
    //cout << "enter beep init" << endl;
    vector<Location *> locs;
    vector<std::string> loc_names;
    PPL::NNC_Polyhedron cvx(cvars.size());

    string ln="";
    for (auto it = automata.begin(); it != automata.end(); it++) {
	    loc_names.push_back(it->get_init_location());
	    locs.push_back(&(it->get_location_by_name(it->get_init_location())));
	    ln += it->get_init_location();
    }
    cvx = NNC_Polyhedron(init_constraint.to_Linear_Constraint(cvars, dvars));

    VariableList dual_cvars;
    Valuations dual_dvars;

    // dual_cvars contains both continuous variables and parameters
    for( auto x : MODEL.get_cvars()) {
        dual_cvars.insert(x);
    }

    for( auto x : dvars) {
        if(MODEL.is_parameter(x.first)) {
            dual_cvars.insert(x.first);
        }
        else {
            dual_dvars.insert(x);
        }
    }

    PPL::NNC_Polyhedron dual_cvx(dual_cvars.size());
    dual_cvx = NNC_Polyhedron(init_constraint.to_Linear_Constraint(dual_cvars, dual_dvars));

    param_region = dual_cvx;
    map_to_parameters(param_region, dual_cvars);
    //cout << "The dual cvx : " << dual_cvx << endl;
    //cout << "The param region : " << param_region << endl << endl << endl;

    shared_ptr<Symbolic_State> beep_init;

    beep_init = make_shared<Dual_Symbolic_State>(loc_names, dvars, cvx, dual_cvx);

    beep_init->continuous_step();

    return beep_init;
}



/************** Corrected codes for path formulation and parameter synthesis *******************************/

void Model::BEEP(const Valuations &pi0)
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
    /** Constraints on unreachable part of an unreachable trace. */
    vector<PPL::NNC_Polyhedron> disjunction_part;

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
                    auto myxp = dynamic_pointer_cast<Dual_Symbolic_State>(x->get_prior());
                    // parameter constraint on the reach part of the trace can be
                    // added into "conjunction_part" directly
                    PPL::NNC_Polyhedron reach = myxp->get_dual_cvx();
                    map_to_parameters(reach, myxp->get_dual_cvars());
                    //////////////////////////////////////////conjunction_part.intersection_assign(reach);
                    conjunction_part.intersection_assign(reach);

                    PPL::NNC_Polyhedron unreach = myx->get_dual_cvx();
                    if( not unreach.is_empty()) {
                        map_to_parameters(unreach, myx->get_dual_cvars());
                        // For the reason of efficiency, we leave the merge of parameter 
                        // synthesis on the unreach part in the further
                        disjunction_part.push_back(unreach);
                    }
                    continue;
                }
                if(x->is_bad()) { // The bad location is met and a bad tile can be returned directly
                    auto myx = dynamic_pointer_cast<Dual_Symbolic_State>(x); 
                    PPL::NNC_Polyhedron bad_cvx = myx->get_dual_cvx();
                    map_to_parameters(bad_cvx, myx->get_dual_cvars());
                    bad_tiles.push_back(bad_cvx);
                    cout << "A bad tile : " << bad_cvx << endl;
                    //BS.push_back(Time_Abstract_State(x));
                    dvars = old;
                    //auto prior = x->get_prior();
                    //while (prior != nullptr) {
                    //    prior->print();
                    //    prior = prior->get_prior();
                    //}
                    return;
                }


                /** To detect a cycle, we backtrack and check if some precedent state contains current state. */
                if( dominated_relation(x, current, dominating, dominated) +
                    dominated_relation(x, next, dominating, dominated) +
                    dominated_relation(x, Space, dominating, dominated) > 0) 
                    continue;


                if(x->no_outgoings()) { // acyclic trace is here
                    auto myx = dynamic_pointer_cast<Dual_Symbolic_State>(x);
                    PPL::NNC_Polyhedron reach = myx->get_dual_cvx();
                    map_to_parameters(reach, myx->get_dual_cvars());
                    conjunction_part.intersection_assign(reach);
                }

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
    PPL::Pointset_Powerset<PPL::NNC_Polyhedron> good_tile(conjunction_part);
    for( auto &x: disjunction_part) {
        good_tile.difference_assign(Pointset_Powerset<NNC_Polyhedron>(x));
    }

    //cout << "good tile before growing parameter constraints " << good_tile << endl;

    for( int i = 0; i < dominating.size(); i++) {
        auto mying = dynamic_pointer_cast<Dual_Symbolic_State>(dominating[i]); 
        auto myed = dynamic_pointer_cast<Dual_Symbolic_State>(dominated[i]); 
        grow_by_steps(good_tile, dominating[i], dominated[i], pi0);
    }
    //for( auto &x: disjunction_part) {
    //    good_tile.difference_assign(Pointset_Powerset<NNC_Polyhedron>(x));
    //}

    good_tiles.push_back(good_tile);
    cout << "A good tile : " << good_tile << endl;
    dvars = old;
}

int Model::dominated_relation(const std::shared_ptr<Symbolic_State> &ss, const std::list<std::shared_ptr<Symbolic_State> > &li, std::vector<shared_ptr<Symbolic_State> >& dominating, std::vector<shared_ptr<Symbolic_State> >& dominated) const
{
    int c = 0;
    for( auto &x : li) {
        if( x->contains(ss)) {
            c++;
            dominating.push_back(x);
            dominated.push_back(ss);
            return c;
        }
    }
    return c;
}

void Model::grow_by_steps(Pointset_Powerset<NNC_Polyhedron> &good_tile, const shared_ptr<Symbolic_State> &dominating, const shared_ptr<Symbolic_State> &dominated, const Valuations &pi0, const double step) const
{
    auto mydominating = dynamic_pointer_cast<Dual_Symbolic_State>(dominating);
    auto mydominated = dynamic_pointer_cast<Dual_Symbolic_State>(dominated);

    const NNC_Polyhedron & dominating_cvx = mydominating->get_dual_cvx();
    const NNC_Polyhedron & dominated_cvx = mydominated->get_dual_cvx();

    // A copy of original cvx we are going to manupilate
    Pointset_Powerset<NNC_Polyhedron> ing(dominating_cvx);
    Pointset_Powerset<NNC_Polyhedron> ed(dominated_cvx);
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

    Pointset_Powerset<NNC_Polyhedron> refined_tile = good_tile;

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
        Pointset_Powerset<NNC_Polyhedron> curr_refined_tile = refined_tile;
        cut_tile(i, i_trial_upper_bound, up_cut_flag, i_lower_bound, down_cut_flag, refined_tile);
        //cout << "refined tile " << refined_tile << endl;

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

                Pointset_Powerset<NNC_Polyhedron> trial_refined_tile = refined_tile;
                cut_tile(i, i_trial_upper_bound, up, i_lower_bound, down, trial_refined_tile);

                ing = Pointset_Powerset<NNC_Polyhedron>(dominating_cvx);
                ed = Pointset_Powerset<NNC_Polyhedron>(dominated_cvx);

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

                Pointset_Powerset<NNC_Polyhedron> trial_refined_tile = refined_tile;
                cut_tile(i, i_upper_bound, up, i_trial_lower_bound, down, trial_refined_tile);

                ing = Pointset_Powerset<NNC_Polyhedron>(dominating_cvx);
                ed = Pointset_Powerset<NNC_Polyhedron>(dominated_cvx);

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

bool Model::be_outside(const int i, const double v, const Pointset_Powerset<NNC_Polyhedron> &domain) const
{
    auto p = parameters[i];
    if( v < p.min or v > p.max)
        return true;

    Pointset_Powerset<NNC_Polyhedron> tmp (domain);
    tmp.add_constraint(Variable(i) == v);
    if( tmp.is_empty())
        return true;
    else
        return false;
}

void Model::cut_tile(const int i, const int i_upper_bound, const bool up, const int i_lower_bound, const bool down, PPL::Pointset_Powerset<PPL::NNC_Polyhedron> &trial_refined_tile) const
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

Parameter Model::valuation_index_to_parameter(const Valuations& v, const int index) const
{
    if( index < 0)
        throw string("valuation_index_to_parameter receives negative index parameter ...");

    int i = 0;
    auto it = v.begin();
    while( i != index) {
        it ++;
        i ++;
    }
    for( auto &x: parameters) {
        if (x.name == it->first)
            return x;
    }
}

void Model::BEEP()
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
        if( in_a_tile(x)) {
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

        BEEP(x);

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
    for( auto &x : good_tiles) {
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

vector<Valuations> Model::increase_by_one_step(const Valuations& v) const
{
    vector<Valuations> vv;
    int index = -1;
    for( auto &x : v) {
        index ++;
        Parameter p = valuation_index_to_parameter(v,index);
        if (x.second == p.max) 
            continue;
        Valuations nu_v = v;
        set_valuation(nu_v, x.first, x.second+1);
        vv.push_back(nu_v);
    }
    return vv;
}

bool Model::existing_point(const Valuations &v, const vector<Valuations> &vv) const
{
    for(auto &x: vv) {
        auto vt = v.begin();
        auto xt = x.begin();
        while( vt != v.end()) {
            if( vt->first != xt->first)
                throw string("Discrete variable order in Valuations does not perserve ...");
            if( vt->second != xt->second)
                break;
            xt ++;
            vt ++;
        }
        if( vt == v.end()) {
            return true;
        }
    }
    return false;
}
