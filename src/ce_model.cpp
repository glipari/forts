
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

  list<shared_ptr<Symbolic_State> > next;
  list<shared_ptr<Symbolic_State> > current;
  Space.clear();

  current.push_back(init);
  
  int step = 0; 
  /**If the CE guided procedure terminates with in the bounded steps,
   * a complete parameter space will be synthesized; otherwise,
   * a under-approximate space will be resulted.
   **/
  bool complete = false;
  
  while (true) {
    for( auto it = current.begin(); it != current.end(); it++) {
      
      if(not (*it)->is_valid())
        continue;
      
      vector<shared_ptr<Symbolic_State> > nsstates = Post(*it);
      
      for( auto iit = nsstates.begin(); iit != nsstates.end(); iit++) {
        (*iit)->mark_prior(*it);
        if( (*iit)->is_empty())
          continue;
        /**
         * Even if "*iit" is not empty, it may become
         * false reachable by taking counter-examples 
         * into account.
         **/
        NNC_Polyhedron poly = (*iit)->get_cvx();
        unconstrain_to_parameters(poly);
        PPL::Pointset_Powerset<PPL::NNC_Polyhedron> pp(poly);
        if(union_ces.contains(pp))
          continue;

        /** The bad location is reached ... */
        if ( (*iit)->is_bad()) {
          union_ces.add_disjunct(poly);
          ces.push_back(poly);
          //refine_with_counter_examples(union_ces, Space);
          refine_with_counter_examples(union_ces, current);
          refine_with_counter_examples(union_ces, next);
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
    if ( bound > 0 and step >= bound)
        break;
    current.splice(current.begin(), next);
  }

    if( not complete) {
      cout << "no complete : " << next.size() << endl;
      int i = 0;
        for ( auto & x : next) {
          auto p = x->get_prior();
          auto tp = p;
          bool found = false;
          while( p != nullptr) {
            if( p->get_loc_names() == x->get_loc_names()) {
              tp = p;
              found = true;
            }
            p = p->get_prior();
          }

          //if( not found)
            //throw string("Failed to track from a amature path ... ");
          const NNC_Polyhedron &sup = x->get_cvx();
          const NNC_Polyhedron &inf = x->get_cvx();
          //map_to_parameters(sup);
          //map_to_parameters(inf);
          list<NNC_Polyhedron> res = get_incl_constraints(inf, sup);
          cout << "To force inclusion : " << i++ << ", " << res.size() << endl;
          ending_points.splice(ending_points.begin(), res);

        }
    }

    cout << "Counter examples : " << ces.size() << endl;
    //for ( auto & x: ces) {
    //    cout << x << endl;
    //}
    //cout << "Counter examples (map to parameters): " << endl;
    //for ( auto & x: ces) {
    //    NNC_Polyhedron y(x);
    //    map_to_parameters(y);
    //    cout << y << endl;
    //}
    cout << "Ending points : " << ending_points.size() << endl;
    //for ( auto & x: ending_points) {
    //    cout << x << endl;
    //}

    //cout << "size of next : " << next.size() << endl;
}

shared_ptr<Symbolic_State> Model::init_param_sstate()
{
  vector<std::string> loc_names;
  
  // The constructor of Symbolic_State needs the initial name of each automaton
  for (auto it = automata.begin(); it != automata.end(); it++) {
    loc_names.push_back(it->get_init_location());
  }
  
  // Parameters are regarded as continuous variables with rate 0
  for( auto &x : parameters) 
    cvars.insert(x.name);
  
  // Never forget to check if the model is consistent
  check_consistency();
  
  PPL::NNC_Polyhedron cvx(cvars.size());
  cvx.add_constraints(init_constraint.to_Linear_Constraint(cvars, dvars));

  PPL::NNC_Polyhedron d_cvx(cvars.size());
  for ( auto & x : parameters) {
    int min = x.min;
    int max = x.max;
    Variable v(get_cvar_index(x.name));
    d_cvx.add_constraint(v>=min);
    d_cvx.add_constraint(v<=max);
  }
    
  // To build the parameter space we're going to explore.
  param_domain = d_cvx;
  cvx.intersection_assign(d_cvx);
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

  union_ces = PPL::Pointset_Powerset<PPL::NNC_Polyhedron>(cvars.size(), EMPTY);

  shared_ptr<Symbolic_State> init = make_shared<Param_Symbolic_State>(loc_names, dvars, cvx);

  // Let's have a continuous step ;)
  init->continuous_step();
  return init;
}    

/**
 * To check if a NNC_Polyhedron is contained in a list.
 **/
bool Model::contained_in(const PPL::NNC_Polyhedron &c, const std::list<NNC_Polyhedron> &lcs)
{
    for (auto &x : lcs)
        if(x.contains(c))
            return true;
    return false;
}

/**
 * To remove elements, which is included in c, from a list.
 **/
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

void Model::refine_with_counter_examples(const PPL::Pointset_Powerset<NNC_Polyhedron>& uc, std::list<std::shared_ptr<Symbolic_State> > &lss)
{
    for( auto&x: lss) {
      if(not x->is_valid())
        continue;
      PPL::NNC_Polyhedron poly(x->get_cvx());
      //unconstrain_to_parameters(poly);
      Pointset_Powerset<NNC_Polyhedron> pp(poly);
      if( uc.contains(pp)) {
         x->invalidate(); 
      }
    }
}

static bool a_feasible_line(const vector<double>& p1, const vector<double>& p2, const vector<vector<double> >&whole) {

  if(p2.at(0) == p1.at(0)) {
    double k = p1.at(0);
    bool positive = false, negative = false;
    for( auto x : whole) {
      if( x.at(0) > k)
        positive = true;
      if( x.at(0) < k)
        negative = true;
      if( negative and positive)
        return false;
    }
    return true;
  }

  double a = (1.0*p2.at(1) - 1.0*p1.at(1))/(p2.at(0)-p1.at(0));
  double b = p1.at(1) - a*p1.at(0);
  bool positive = false, negative = false;
  for( auto x : whole) {
    if( a*x.at(0)+b > x.at(1)) {
      positive = true;
    }
    if( a*x.at(0)+b < x.at(1)) {
      negative = true;
    }
    if( negative and positive)
      return false;
  }
  return true;

}


void Model::print_points_ce(string fname) const
{
    vector<Parameter> parameters1(parameters);
    sort(parameters1.begin(), parameters1.end());
    string X = parameters1[0].name;
    string Y = parameters1[1].name;
    string graph_command = string("graph") + string(" -T ps -C -X ") + X + string(" -Y ") + Y + string(" ");

    int index = 0;
    
    // To output the parameter domain
    string output = fname + string("param_domain");
    graph_command += string("-m7 -q 1 ") + output +string(" ");
    NNC_Polyhedron d_cvx(param_region);
    map_to_parameters(d_cvx);
    print_cvx(d_cvx, output);

    // to print bad parameters

    for( auto &x : ending_points) {
      string output = fname + string("-unclear-tile-") + to_string(index++);
      //NNC_Polyhedron y(x);
      print_cvx(x, output);
      graph_command += string(" -m 10 -q 1 ")  + output + string(" ");
    }
    //NNC_Polyhedron y(cvars.size());
    //output = fname + string("-good-tile-");
    //for( auto &x : ending_points) {
    //  y.intersection_assign(x);
    //  if(y.is_empty())
    //    throw string("Empty good space ...");
    //}
    //graph_command += string(" -m 2 -q 1 ")  + output + string(" ");
    //map_to_parameters(y);
    //if( ending_points.size() !=0 )
    //  print_cvx(y, output);

    index = 0;
    for( auto &x : ces) {
      string output = fname + string("-bad-tile-") + to_string(index++);
      NNC_Polyhedron y(x);
      map_to_parameters(y);
      print_cvx(y, output);
      graph_command += string(" -m 6 -q 1 ")  + output + string(" ");
    }

    string graph_output = fname + string(".ps");
    graph_command += string(" -L \"\" > ") + graph_output;
    string graph_command_fname = fname+string("-graph.sh");
    ofstream graph_ofs;
    graph_ofs.open(graph_command_fname.c_str());
    graph_ofs << graph_command;
}

int Model::get_cvar_index(const string& s) const {
  int index = 0;
  for ( auto x : cvars) {
    if ( x == s)
      return index;
    index ++;
  }
  throw string("Undefined continuous variable ") + s;

}

void Model::print_cvx(const PPL::NNC_Polyhedron &cvx, const std::string &fname) const {
      //cout << fname << " : " << cvx << endl;
      /**
       * This is actually "cheating" by manually changing NNC_Polyhedron to C_Polyhedron... 
       **/ 
      PPL::C_Polyhedron cpp(cvx);
      const PPL::Generator_System &gs = cpp.generators();

      ofstream ofs;
      //string output = fname + string("-bad-tile-") + to_string(index++);
      ofs.open(fname.c_str());

      vector< vector<double> > points;
      for( auto jt = gs.begin(); jt != gs.end(); jt ++) {
        vector<double> point;
        for( auto dim = jt->space_dimension(); dim -- > 0;) {
          stringstream ss1 (stringstream::in | stringstream::out);
          stringstream ss2 (stringstream::in | stringstream::out);
          
          ss1 << jt->coefficient(PPL::Variable(dim));
          ss2 << jt->divisor();
          double coe, div;
          ss1 >> coe; ss2 >> div;
          point.push_back(coe/div);
          reverse(point.begin(), point.end());
        }
        points.push_back(point);
      }

      vector<vector<double> > ps(points);
      points.clear();
      points.push_back(ps.front());
      ps.erase(ps.begin());
      
      while( ps.size() != 0) {
        
        vector<double> p1(points.back());
        
        auto it = ps.begin();
        while (it != ps.end()) {
          vector<double> p2(*it);
          vector<vector<double> > whole(points);
          whole.pop_back();
          for(auto xt = ps.begin(); xt!= ps.end(); xt++) {
            if( xt == it)
              continue;
            whole.push_back(*xt);
          }
          
          if( a_feasible_line(p1, p2, whole)) {
            points.push_back(p2);
            ps.erase(it);
            break;
          }
          else it ++;
            
          if( it == ps.end())
            throw("sorting exception");
        }
      }

      points.push_back(points[0]);
      for( auto &x: points) {
        for( int i = 0; i < x.size(); i++) {
          if( i != 0) ofs << " ";
          ofs << x[i];
        }
        ofs << endl;
      }
}
    
std::list<PPL::NNC_Polyhedron> Model::get_incl_constraints(const PPL::NNC_Polyhedron& inf, const PPL::NNC_Polyhedron& sup) const
{
  cout << "enter get incl constraints " << endl;
  int s = inf.space_dimension();
  int ss = sup.space_dimension();
  if (s != ss)
    throw string("Different space dimensions in get_incl_constraints");
  list<NNC_Polyhedron> res;

  int index = -1;
  NNC_Polyhedron inf1(inf);
  NNC_Polyhedron inf2(inf);
  //inf1.concatenate_assign(sup);
  //inf2.concatenate_assign(sup);
  //for (auto x : cvars) {
  //  cout << "cvar : " << x << endl;
  //  index ++;
  //  if (is_parameter(x)) {
  //    inf1.add_constraint(Variable(index)==Variable(index+s));
  //    inf2.add_constraint(Variable(index)==Variable(index+s));
  //    continue;
  //  }

  //  inf1.add_constraint(Variable(index)==Variable(index+s));
  //  inf2.add_constraint(Variable(index)==Variable(index+s));

  //}

  //inf1.remove_higher_space_dimensions(s);
  //cout << "inf1 : " << inf1 << endl;
  //inf2.remove_higher_space_dimensions(s);
  //cout << "inf2 : " << inf2 << endl;

  //inf1.intersection_assign(inf2);
  //cout << "inf1 and inf2 : " << inf1 << endl;

  map_to_parameters(inf1);
  if( inf1.is_empty())
    throw string("Empty region returned in get_incl_constraints method.");

  res.push_back(inf1);

  return res;

}

