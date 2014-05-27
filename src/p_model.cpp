#include "model.hpp"
#include "common.hpp"

using namespace std;
using namespace Parma_Polyhedra_Library::IO_Operators;

/** Model with parameters. */
Parameter::Parameter(string n, int mi, int ma) : name(n), min(mi), max(ma) {}

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

void Model::BEEP()
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

        if( BS.size() == 0)
            build_a_good_tile();
        else
            build_a_bad_tile();
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

void Model::BEEP(const Valuations &pi0)
{
    Valuations old = dvars;
    dvars.insert(pi0.begin(), pi0.end());
    //for ( auto x:dvars) {
    //    cout << x.first << " " << x.second << endl;
    //}

    set_sstate_type(ORIGIN);

    shared_ptr<Symbolic_State> init = init_sstate();
    list<shared_ptr<Symbolic_State> > next;
    list<shared_ptr<Symbolic_State> > current;
    current.push_back(init);
    int step = 0;

    while( true) {
        for (auto it = current.begin(); it != current.end(); it++) {
            vector<shared_ptr<Symbolic_State> > nsstates = Post(*it);
            for ( auto &x : nsstates) {
                x->mark_prior(*it);
                if (x->is_empty()) {
                    UNReach.push_back(Time_Abstract_State(x));
                    continue;
                }
                if(x->is_bad()) {
                    BS.push_back(Time_Abstract_State(x));
                    dvars = old;
                    return;
                }

                if(contained_in_then_store(x, current)) continue;
                if(contained_in_then_store(x, next)) continue;
                if(contained_in_then_store(x, Space)) continue;

                if(x->no_outgoings()) {
                    Acyclic.push_back(Time_Abstract_State(x));
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

    //cout << "---------------------------------------------------" << endl;
    //cout << "UNReach    : " << UNReach.size() << endl;
    //cout << "Acyclic    : " << Acyclic.size() << endl;
    //cout << "Contained  : " << Contained.size() << endl;
    //cout << "BS         : " << BS.size() << endl;
    //cout << "---------------------------------------------------" << endl;
    dvars = old;
}

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

void Model::build_a_good_tile()
{
    VariableList old = cvars;
    for( auto &x : parameters) {
        cvars.insert(x.name);
    }

    PPL::Pointset_Powerset<PPL::NNC_Polyhedron> tile(parameters.size());
    //cout << "tile-UNReach : " << tile << endl;
    for(auto &x : UNReach) {
        //cout << "---------------------------------------------------" << endl;
        //x.print();
        // an unfeasible trace
        const vector<Combined_edge> &trace1 = x.get_trace();
        // the feasible prefix
        vector<Combined_edge> trace2 = trace1;
        trace2.pop_back();

        NNC_Polyhedron cvx2 = trace_to_cvx(trace2);
        //cout << "cvx2 before mapping : " << cvx2 << endl;
        map_to_parameters(cvx2);
        //cout << "cvx2-UNReach : " << cvx2 << endl;
        tile.intersection_assign(PPL::Pointset_Powerset<PPL::NNC_Polyhedron>(cvx2));
        //cout << "tile-UNReach : " << tile << endl;

        NNC_Polyhedron cvx1 = trace_to_cvx(trace1);
        map_to_parameters(cvx1);
        //cout << "cvx1-UNReach : " << cvx1 << endl;
        //cout << "---------------------------------------------------" << endl;
        if(cvx1.is_empty())
            continue;
        tile.difference_assign(PPL::Pointset_Powerset<PPL::NNC_Polyhedron>(cvx1));
        //cout << "tile-UNReach : " << tile << endl;

    }
     
    for(auto &x : Contained) {
        // a feasible trace
        const vector<Combined_edge> &trace1 = x.first.get_trace();
        // an unfeasible trace
        const vector<Combined_edge> &trace2 = x.second.get_trace();

        NNC_Polyhedron cvx1 = trace_to_cvx(trace1);

        NNC_Polyhedron cvx2 = trace_to_cvx(trace2);

        // cvx1 is included in cvx2
        cvx2.concatenate_assign(cvx1);
        cvx2.concatenate_assign(cvx1);
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
        tile.intersection_assign(PPL::Pointset_Powerset<PPL::NNC_Polyhedron>(cvx2));
        //cout << "tile-Contained : " << tile << endl;
    }

    for(auto &x : Acyclic) {
        auto trace = x.get_trace();
        NNC_Polyhedron cvx = trace_to_cvx(trace);
        map_to_parameters(cvx);
        tile.intersection_assign(PPL::Pointset_Powerset<PPL::NNC_Polyhedron>(cvx));

    }
    
    cout << "a good tile : " << tile << endl;

    cvars = old;
    good_tiles.push_back(tile);
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

NNC_Polyhedron Model::trace_to_cvx(const vector<Combined_edge>& tr)
{
    shared_ptr<Symbolic_State> sstate = init_sstate();
    //cout << "Init : " ;
    //sstate->print();
    for( auto &x: tr) {
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
