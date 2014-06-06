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
//Parameter::Parameter(string n, int mi, int ma) : name(n), min(mi), max(ma) {}

//Parameter::Parameter() {}

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

void Model::BEEP1()
{
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

    vector<Valuations> points = generate_valuations_list(parameters);
    //for( auto &x: points) {
    //    for( auto &y: x) {
    //        cout << y.first << "==" << y.second << "  ";
    //    }
    //    cout << endl;
    //}

    for( auto &x: points){
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
            continue;
        }
        cout << "is not contained in a generated tile." << endl;

        BEEP(x);
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

//void Model::BEEP(const Valuations &pi0)
//{
//    Valuations old = dvars;
//    dvars.insert(pi0.begin(), pi0.end());
//    //for ( auto x:dvars) {
//    //    cout << x.first << " " << x.second << endl;
//    //}
//
//    set_sstate_type(ORIGIN);
//
//    shared_ptr<Symbolic_State> init = init_sstate();
//    list<shared_ptr<Symbolic_State> > next;
//    list<shared_ptr<Symbolic_State> > current;
//    current.push_back(init);
//    int step = 0;
//
//    while( true) {
//        for (auto it = current.begin(); it != current.end(); it++) {
//            vector<shared_ptr<Symbolic_State> > nsstates = Post(*it);
//            for ( auto &x : nsstates) {
//                x->mark_prior(*it);
//                if (x->is_empty()) {
//                    UNReach.push_back(Time_Abstract_State(x));
//                    continue;
//                }
//                if(x->is_bad()) {
//                    BS.push_back(Time_Abstract_State(x));
//                    dvars = old;
//                    return;
//                }
//
//                if(contained_in_then_store(x, current)) continue;
//                if(contained_in_then_store(x, next)) continue;
//                if(contained_in_then_store(x, Space)) continue;
//
//                if(x->no_outgoings()) {
//                    Acyclic.push_back(Time_Abstract_State(x));
//                }
//
//                next.push_back(x);
//
//            }
//        }
//        Space.splice(Space.end(), current);
//        //cout << "---------------------------------------------------" << endl;
//        //cout << "Step : " << ++step << endl;
//        //cout << "Number of passed states : " << Space.size() + current.size()<< endl;
//        //cout << "Number of generated states : " << next.size() << endl;
//        //cout << "---------------------------------------------------" << endl;
//        if( next.size() == 0) {
//            break;
//        }
//        current.splice(current.end(), next);
//
//    }
//
//    cout << "---------------------------------------------------" << endl;
//    cout << "UNReach    : " << UNReach.size() << endl;
//    cout << "Acyclic    : " << Acyclic.size() << endl;
//    cout << "Contained  : " << Contained.size() << endl;
//    cout << "BS         : " << BS.size() << endl;
//    cout << "---------------------------------------------------" << endl;
//    dvars = old;
//}

bool Model::contained_in_then_store(const shared_ptr<Symbolic_State> &ss, const list<shared_ptr<Symbolic_State> > &lss)
{
    for ( auto it = lss.begin(); it != lss.end(); it++) {
	    bool f = (*it)->contains(ss);
	    if (f) {
            Contained.push_back(pair<Time_Abstract_State,Time_Abstract_State>(Time_Abstract_State(ss),Time_Abstract_State(*it)));
            return true;
        }
    }
    return false;
}

void Model::build_a_bad_tile()
{
    VariableList old = cvars;
    for( auto &x : parameters) {
        cvars.insert(x.name);
    }

    PPL::NNC_Polyhedron tile(parameters.size());
    for(auto &x : BS) {
        auto trace = x.get_trace();
        //x.print();
        NNC_Polyhedron cvx = trace_to_cvx(trace);
        //cout << "cvx : " << cvx << endl;
        map_to_parameters(cvx);
        tile.intersection_assign(PPL::NNC_Polyhedron(cvx));

    }
    //cout << tile << endl;
    cout << "a bad tile : " << tile << endl;

    cvars = old;
    bad_tiles.push_back(tile);
}

pair<NNC_Polyhedron, NNC_Polyhedron> Model::unreach_trace_to_cvx(const Trace& tr)
{
    shared_ptr<Symbolic_State> sstate = init_sstate();
    NNC_Polyhedron cvx1, cvx2;
    int pos = 0;
    auto ces = tr.get_combined_edges();
    for( auto &x: ces) {
        if( pos ++ == ces.size() - 1)
            cvx1 = sstate->get_cvx();
        sstate->discrete_step(x);
        sstate->continuous_step();
    }
    cvx2 = (sstate->get_cvx());
    return pair<NNC_Polyhedron,NNC_Polyhedron>(cvx1, cvx2);
}
    

NNC_Polyhedron Model::trace_to_cvx(const Trace& tr)
{
    shared_ptr<Symbolic_State> sstate = init_sstate();
    //cout << "Init : " ;
    //sstate->print();
    auto ces = tr.get_combined_edges();
    for( auto &x: ces) {
        sstate->discrete_step(x);
        sstate->continuous_step();
        //sstate->print();
    }
    PPL::NNC_Polyhedron poly(sstate->get_cvx());
    //PPL::Variables_Set vars;
    //int pos = 0;
    //for( auto &x:cvars) {
    //    //cout << x << endl;
    //    if(not is_parameter(x))
    //        vars.insert(PPL::Variable(pos));
    //    pos++;
    //}
    //poly.remove_space_dimensions(vars);
    //cout << "poly " << poly << endl;
    return poly;
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
    int index = 0;
    // to print good tiles
    for( auto &x : good_tiles) {
        for (auto it = x.begin(); it != x.end(); it++) {
            const PPL::NNC_Polyhedron &cp = it->pointset(); 
            const PPL::Generator_System &gs = cp.generators();

            ofstream ofs;
            string output = fname + string("-good-tile-") + to_string(index++);
            ofs.open(output.c_str());

            vector< vector<double> > points;
            for( auto jt = gs.begin(); jt != gs.end(); jt ++) {
                if( !jt->is_point()) throw ("Not a point here ...");
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
                for( int i = 0; i < x.size(); i++) {
                    if( i != 0) ofs << " ";
                    ofs << x[i];
                }
                ofs << endl;
            }
        }
    }
    
    // to print bad tiles
    index = 0;
    for( auto &x : bad_tiles) {
        const PPL::Generator_System &gs = x.generators();

        ofstream ofs;
        string output = fname + string("-bad-tile-") + to_string(index++);
        ofs.open(output.c_str());

        vector< vector<double> > points;
        for( auto jt = gs.begin(); jt != gs.end(); jt ++) {
            if( !jt->is_point()) throw ("Not a point here ...");
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
            for( int i = 0; i < x.size(); i++) {
                if( i != 0) ofs << " ";
                ofs << x[i];
            }
            ofs << endl;
        }
    }
}

void Model::build_a_good_tile()
{
    VariableList old = cvars;
    for( auto &x : parameters) {
        cvars.insert(x.name);
    }

    vector<PPL::NNC_Polyhedron> tiles;
    vector<PPL::NNC_Polyhedron> unreach_tiles;
    PPL::NNC_Polyhedron tile(parameters.size());
    for(auto &x : Contained) {
        // a feasible trace
        auto trace1 = x.first.get_trace();
        // an unfeasible trace
        auto trace2 = x.second.get_trace();

        NNC_Polyhedron cvx1 = trace_to_cvx(trace1);

        NNC_Polyhedron cvx2 = trace_to_cvx(trace2);

        // cvx1 is included in cvx2
        cvx2.concatenate_assign(cvx1);
        //cvx2.concatenate_assign(cvx1);
        int pos = 0;
        PPL::Variables_Set vars;
        for( auto &x:cvars) {
            cvx2.add_constraint(PPL::Variable(pos) == PPL::Variable(pos+cvars.size()));
            if( not is_parameter(x))
                vars.insert(PPL::Variable(pos));
            pos ++;
        }
        cvx2.remove_higher_space_dimensions(cvars.size());
        cvx2.remove_space_dimensions(vars);
        //cout << "cvx2-Contained : " << cvx2 << endl;
        tile.intersection_assign(cvx2);
        //cout << "tile-Contained : " << tile << endl;
    }

    for(auto &x : Acyclic) {
        auto trace = x.get_trace();
        NNC_Polyhedron cvx = trace_to_cvx(trace);
        map_to_parameters(cvx);
        tile.intersection_assign(cvx);

    }

    cout << "Reachable part is done : " << tile << endl; 
    for(auto &x : UNReach) {
        auto trace = x.get_trace();
        //vector<Combined_edge> trace2 = trace1;
        //trace2.pop_back();

        NNC_Polyhedron cvx1, cvx2;

        auto res = unreach_trace_to_cvx(trace);
        cvx1 = res.first;
        cvx2 = res.second;
        //NNC_Polyhedron cvx2 = trace_to_cvx(trace2);
        map_to_parameters(cvx1);
        tile.intersection_assign(cvx1);

        //NNC_Polyhedron cvx1 = trace_to_cvx(trace1);
        map_to_parameters(cvx2);
        if(cvx2.is_empty())
            continue;
        unreach_tiles.push_back(cvx2);
        //tile.difference_assign(PPL::Pointset_Powerset<PPL::NNC_Polyhedron>(cvx1));

    }

    PPL::Pointset_Powerset<PPL::NNC_Polyhedron> good_tile(tile);
    //cout << "UNReachable part is done : " << tile << endl; 
    cout << "Unreachable states : " << unreach_tiles.size() << endl;

    for( auto &x: unreach_tiles)
        good_tile.difference_assign(PPL::Pointset_Powerset<NNC_Polyhedron>(x)); 
    
    cout << "a good tile : " << good_tile << endl;

    cvars = old;
    good_tiles.push_back(good_tile);
}

/////////////////////// breadth-first parameter synthesis //////////////////////////////////
//void Model::build_a_good_tile_c()
//{
//    VariableList old = cvars;
//    for( auto &x : parameters) {
//        cvars.insert(x.name);
//    }
//
//    set_sstate_type(ORIGIN);
//
//    shared_ptr<Symbolic_State> init = init_sstate();
//    Space.clear();
//    list<shared_ptr<Symbolic_State> > next;
//    list<shared_ptr<Symbolic_State> > current;
//    current.push_back(init);
//    //int step = 0;
//    
//
//    NNC_Polyhedron reach(parameters.size());
//    vector<NNC_Polyhedron> unreach;
//
//    while( true) {
//        bool done = false;
//        for (auto it = current.begin(); it != current.end(); it++) {
//            vector<shared_ptr<Symbolic_State> > nsstates = Post(*it);
//            for ( auto &x : nsstates) {
//                x->mark_prior(*it);
//                
//                //Time_Abstract_State tas_x(x);
//                int left =  try_addC_and_check_ending(x, reach, unreach); 
//                //cout << left << "...";
//                if( left == 0) {
//                //if( try_addC_and_check_ending(x, reach, unreach) == 0) {
//                    done = true;
//                    break;
//                }
//
//                if (x->is_empty()) {
//                    continue;
//                }
//
//                if(contained_in(x, current)) continue;
//                if(contained_in(x, next)) continue;
//                if(contained_in(x, Space)) continue;
//
//                next.push_back(x);
//
//            }
//        }
//    //cout << "---------------------------------------------------" << endl;
//    //cout << "Space    : " << Space.size() << endl;
//    //cout << "current    : " << current.size() << endl;
//    //cout << "next  : " << next.size() << endl;
//    //cout << "---------------------------------------------------" << endl;
//        Space.splice(Space.end(), current);
//        if( done) {
//            break;
//        }
//        if( next.size() == 0)
//            break;
//        current.splice(current.end(), next);
//
//    }
//
//    PPL::Pointset_Powerset<PPL::NNC_Polyhedron> good_tile(reach);
//    //cout << "UNReachable part is done : " << tile << endl; 
//    //cout << "Unreachable states : " << unreach.size() << endl;
//
//    for( auto &x: unreach)
//        good_tile.difference_assign(PPL::Pointset_Powerset<NNC_Polyhedron>(x)); 
//
//    good_tiles.push_back(good_tile);
//        
//    cout << "To print ... ";
//    cout << "A good tile : " << good_tile << endl;
//    //cout << "---------------------------------------------------" << endl;
//    //cout << "UNReach    : " << UNReach.size() << endl;
//    //cout << "Acyclic    : " << Acyclic.size() << endl;
//    //cout << "Contained  : " << Contained.size() << endl;
//    //cout << "BS         : " << BS.size() << endl;
//    //cout << "---------------------------------------------------" << endl;
//    cvars = old;
//}
//
//int Model::try_addC_and_check_ending(const shared_ptr<Symbolic_State>& ss, NNC_Polyhedron&reach, vector<NNC_Polyhedron> &unreach)
//{
//    Time_Abstract_State tas(ss);
//    auto tr = tas.get_trace();
//
//    auto it = UNReach.begin();
//    while( it != UNReach.end()) {
//        auto tr1 = it->get_trace();
//        if( tr1 == tr) { // an unreach run is matched
//            auto prior = ss->get_prior();
//            NNC_Polyhedron cvx1 = prior->get_cvx();
//            map_to_parameters(cvx1);
//            reach.intersection_assign(cvx1);
//            NNC_Polyhedron cvx2 = ss->get_cvx();
//            if( not cvx2.is_empty()) {
//                map_to_parameters(cvx2);
//                unreach.push_back(cvx2);
//            }
//            it = UNReach.erase(it);
//        }
//        else { it ++; }     
//    }
//
//    auto jt = Contained.begin();
//    while( jt != Contained.end()) {
//       auto tr2 = jt->second.get_trace();
//       if( tr2 == tr) {
//            auto prior = ss->get_prior();
//            jt->second.set_cvx(prior->get_cvx());
//       }
//       
//       auto tr1 = jt->first.get_trace();
//       if( tr == tr2) {
//            auto prior = ss->get_prior();
//            NNC_Polyhedron cvx2 = jt->second.get_cvx();
//            NNC_Polyhedron cvx1 = prior->get_cvx();;
//
//            cvx2.concatenate_assign(cvx1);
//            //cvx2.concatenate_assign(cvx1);
//            int pos = 0;
//            PPL::Variables_Set vars;
//            for( auto &x:cvars) {
//                cvx2.add_constraint(PPL::Variable(pos) == PPL::Variable(pos+cvars.size()));
//                if( not is_parameter(x))
//                    vars.insert(PPL::Variable(pos));
//                pos ++;
//            }
//            cvx2.remove_higher_space_dimensions(cvars.size());
//            cvx2.remove_space_dimensions(vars);
//            reach.intersection_assign(cvx2);
//            jt = Contained.erase(jt);
//            continue;
//       }
//
//       jt ++;
//    }
//
//    return UNReach.size() + Contained.size();
//}

void Model::on_the_fly_BEEP(const Valuations &pi0)
{
    Valuations old = dvars;
    dvars.insert(pi0.begin(), pi0.end());
    //for ( auto x:dvars) {
    //    cout << x.first << " " << x.second << endl;
    //}


    shared_ptr<Symbolic_State> init = beep_init_sstate();
    list<shared_ptr<Symbolic_State> > next;
    list<shared_ptr<Symbolic_State> > current;
    current.push_back(init);

    PPL::NNC_Polyhedron conjunction_part(parameters.size());
    vector<PPL::NNC_Polyhedron> disjunction_part;

    int step = 0;

    while( true) {
        for (auto it = current.begin(); it != current.end(); it++) {
            vector<shared_ptr<Symbolic_State> > nsstates = Post(*it);
            for ( auto &x : nsstates) {
                x->mark_prior(*it);
                if (x->is_empty()) {
                    auto myx = dynamic_pointer_cast<Dual_Symbolic_State>(x);
                    auto myxp = dynamic_pointer_cast<Dual_Symbolic_State>(x->get_prior());
                    PPL::NNC_Polyhedron reach = myxp->get_dual_cvx();
                    map_to_parameters(reach, myxp->get_dual_cvars());
                    conjunction_part.intersection_assign(reach);
                    PPL::NNC_Polyhedron unreach = myx->get_dual_cvx();
                    if( not unreach.is_empty()) {
                        map_to_parameters(unreach, myx->get_dual_cvars());
                        disjunction_part.push_back(unreach);
                    }
                    continue;
                }
                if(x->is_bad()) {
                    auto myx = dynamic_pointer_cast<Dual_Symbolic_State>(x); 
                    PPL::NNC_Polyhedron bad_cvx = myx->get_dual_cvx();
                    map_to_parameters(bad_cvx, myx->get_dual_cvars());
                    bad_tiles.push_back(bad_cvx);
                    cout << "A bad tile : " << bad_cvx << endl;
                    //BS.push_back(Time_Abstract_State(x));
                    dvars = old;
                    return;
                }

                if(contained_in_then_store(x, current, conjunction_part)) continue;
                if(contained_in_then_store(x, next, conjunction_part)) continue;
                if(contained_in_then_store(x, Space, conjunction_part)) continue;

                if(x->no_outgoings()) {
                    auto myx = dynamic_pointer_cast<Dual_Symbolic_State>(x);
                    PPL::NNC_Polyhedron reach = myx->get_dual_cvx();
                    map_to_parameters(reach, myx->get_dual_cvars());
                    conjunction_part.intersection_assign(reach);
                }

                next.push_back(x);

            }
        }
        Space.splice(Space.end(), current);
        //cout << "---------------------------------------------------" << endl;
        //cout << "Step : " << ++step << endl;
        //cout << "Number of passed states : " << Space.size() + current.size()<< endl;
        //cout << "Number of generated states : " << next.size() << endl;
        //cout << "---------------------------------------------------" << endl;
        if( next.size() == 0) {
            break;
        }
        current.splice(current.end(), next);

    }

    cout << "---------------------------------------------------" << endl;
    cout << "Number of states    : " << Space.size() << endl;
    //cout << "Acyclic    : " << Acyclic.size() << endl;
    //cout << "Contained  : " << Contained.size() << endl;
    //cout << "BS         : " << BS.size() << endl;
    cout << "---------------------------------------------------" << endl;

    PPL::Pointset_Powerset<PPL::NNC_Polyhedron> good_tile(conjunction_part);
    for( auto &x: disjunction_part) {
        //cout << "x : " << x << endl;
        good_tile.difference_assign(Pointset_Powerset<NNC_Polyhedron>(x));
        //cout << "good tile after df : " << good_tile << endl;
        //PPL::Pointset_Powerset<PPL::NNC_Polyhedron> neg(cvars.size()+parameters.size(), EMPTY);
        //// maybe difference assign is not a good way to negate a NNC_Polyhedron x
        ////good_tile.difference_assign(PPL::Pointset_Powerset<NNC_Polyhedron>(x)); 
        //// we are going to negate every inequality in x manually
        //for( auto &c : x.constraints()) {
        //    //cout << "c : " << c << endl;
        //    Constraint cc = c;
        //    Linear_Expression e;
        //    for( PPL::dimension_type i = cc.space_dimension(); i-->0;) {
        //       e += cc.coefficient(Variable(i))*Variable(i);
        //    }
        //    e += cc.inhomogeneous_term();
        //    if( cc.is_equality()) {
        //            //cout << "equality : " << cc << endl;
        //            Constraint cc2 = (e<0);
        //            //cout << "cc2 " << cc2 << endl;
        //            neg.add_disjunct(NNC_Polyhedron(Constraint_System(cc2)));
        //            Constraint cc3 = (e>0);
        //            //cout << "cc3 " << cc3 << endl;
        //            neg.add_disjunct(NNC_Polyhedron(Constraint_System(cc3)));
        //      //      cout << "neg : " << neg << endl;
        //    }
        //    else {

        //        //cout << "inequality : " << cc << endl;
        //        Constraint cc2 = cc.is_strict_inequality() ? (e<=0) : (e<0);
        //        neg.add_disjunct(NNC_Polyhedron(Constraint_System(cc2)));
        //        //    cout << "neg : " << neg << endl;
        //    }
        //}
        ////cout << "final neg: " << neg << endl;
        ////cout << "good tile : " << good_tile << endl;
        //good_tile.intersection_assign(neg);
        ////cout << "after difference good tile : " << good_tile << endl;
    }
    //auto myinit = dynamic_pointer_cast<Dual_Symbolic_State>(init); 
    //cout << "A good tile before map: " << good_tile << endl;
    //map_to_parameters(good_tile, myinit->get_dual_cvars()); 
    good_tiles.push_back(good_tile);
    cout << "A good tile : " << good_tile << endl;
    dvars = old;
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

            //cvx1.intersesction_assign(conjunction_part);
            //cvx2.intersesction_assign(conjunction_part);
            //if(

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
    for( auto x : dvars) {
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

    PPL::NNC_Polyhedron dual_cvx(dual_cvars.size());
    dual_cvx = NNC_Polyhedron(init_constraint.to_Linear_Constraint(dual_cvars, dual_dvars));

    //cout << "dual cvx " << dual_cvx << endl;



    shared_ptr<Symbolic_State> beep_init;

    beep_init = make_shared<Dual_Symbolic_State>(loc_names, dvars, cvx, dual_cvx);

    beep_init->continuous_step();
    //cout << "end of beep init" << endl;
    //beep_init->print();
    return beep_init;
}

void Model::beep_set_on_the_fly()
{
    on_the_fly = true;
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
                    return;
                }

                //if(contained_in_then_store(x, current, conjunction_part, dominating, dominated)) continue;
                //if(contained_in_then_store(x, next, conjunction_part,dominating, dominated)) continue;
                //if(contained_in_then_store(x, Space, conjunction_part, dominating, dominated)) continue;

                /** To detect a cycle, we backtrack and check if some precedent state contains current state. */
                //if( backtrack(x, dominating, dominated))
                    //continue;
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
        //cout << "---------------------------------------------------" << endl;
        //cout << "Step : " << ++step << endl;
        //cout << "Number of passed states : " << Space.size() + current.size()<< endl;
        //cout << "Number of generated states : " << next.size() << endl;
        //cout << "---------------------------------------------------" << endl;
        if( next.size() == 0) {
            break;
        }
        current.splice(current.end(), next);

    }

    cout << "---------------------------------------------------" << endl;
    cout << "Number of states    : " << Space.size() << endl;
    //cout << "Acyclic    : " << Acyclic.size() << endl;
    cout << "Domination  : " << dominated.size() << endl;
    //cout << "BS         : " << BS.size() << endl;
    cout << "---------------------------------------------------" << endl;

    PPL::Pointset_Powerset<PPL::NNC_Polyhedron> good_tile(conjunction_part);
    for( auto &x: disjunction_part) {
        good_tile.difference_assign(Pointset_Powerset<NNC_Polyhedron>(x));
    }

    //cout << "good tile before growing parameter constraints " << good_tile << endl;

    for( int i = 0; i < dominating.size(); i++) {
        auto mying = dynamic_pointer_cast<Dual_Symbolic_State>(dominating[i]); 
        auto myed = dynamic_pointer_cast<Dual_Symbolic_State>(dominated[i]); 
        //cout << "   dominating: " << mying->get_dual_cvx() << endl;
        //cout << "   dominated: " << myed->get_dual_cvx() << endl;
        grow_by_steps(good_tile, dominating[i], dominated[i], pi0);
    }

    //auto myinit = dynamic_pointer_cast<Dual_Symbolic_State>(init); 
    //cout << "A good tile before map: " << good_tile << endl;
    //map_to_parameters(good_tile, myinit->get_dual_cvars()); 
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

//bool Model::backtrack(const std::shared_ptr<Symbolic_State> &ss, std::vector<shared_ptr<Symbolic_State> >& dominating, std::vector<shared_ptr<Symbolic_State> >& dominated) const
//{
//    auto p = ss->get_prior();
//    while ( p != nullptr) {
//        if( p->contains(ss)) {
//            dominating.push_back(p);
//            dominated.push_back(ss);
//            return true;
//        }
//        p = p->get_prior();
//    }
//
//    return false;
//}

        
        
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
        }
        j++;
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
                    }
                    j++;
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
                    }
                    j++;
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
