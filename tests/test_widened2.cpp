#include "catch.hpp" 

#include <fstream>
#include <iostream>
#include <string>
#include <model.hpp>
#include <model_parser.hpp>
#include <constraint_parser.hpp>
#include <box_widened_sstate.hpp>

using namespace Parma_Polyhedra_Library::IO_Operators;
using namespace std;
using namespace tipa;

//    {
//        //widen();
//    }
//
//    TEST_Box_Widened_Symbolic_State(const std::vector<std::string> &loc_names, 
//                                   const Valuations &dvars, const PPL::NNC_Polyhedron &pol) 
//                : Box_Widened_Symbolic_State(loc_names, dvars, pol)
//    {
//        //widen();
//    }
//
//    virtual std::shared_ptr<Symbolic_State> clone() {
//        return make_shared<Symbolic_State> (*this);
//    }
//    const TBox& get_box() const {
//        return box_widened_cvx;
//    }
//
//};
//
//static Location &parse_location(const std::string &auto_loc)
//{
//    std::string a, l;
//
//    tipa::rule name = tipa::rule(tk_ident) >> 
//	tipa::rule('.') >> tipa::rule(tk_ident);
//    
//    auto f = [&a, &l](parser_context &pc) {
//	auto v = pc.collect_tokens();
//	if (v.size() != 2) throw tipa::parse_exc("malformed location name");
//	a = v[0].second;
//	l = v[1].second;
//    };
//    name[ f ];
//    stringstream str {auto_loc};
//    parser_context pc;
//    pc.set_stream(str);
//    name.parse(pc);
//    
//    automaton &at = MODEL.get_automaton_by_name(a);
//    Location  &lc = at.get_location_by_name(l);
//    return lc;
//}
//
//
//static shared_ptr<Symbolic_State> build_state(const std::vector<std::string> &locs, 
//			   const Valuations  &dv,
//			   const std::string &constraints)
//{
//    auto cv = MODEL.get_cvars();
//    auto cs = build_a_constraint_tree(constraints);
//    PPL::NNC_Polyhedron cvx(cv.size());
//    cvx.add_constraints(cs.to_Linear_Constraint(cv, dv));
//    auto res = make_shared<TEST_Box_Widened_Symbolic_State> (locs, dv, cvx );
//    res->continuous_step();
//    res->widen();
//    return res;
//    
//}
//
//static bool compare_state_sets(const list<shared_ptr<Symbolic_State> > &la,
//		       const list<Symbolic_State>  &lb)
//{
//    int c = 0;
//    for (auto &x : la) 
//	if (find(begin(lb), end(lb), *x) != end(lb)) {
//	    c++;
//    }
//	else {
//	    cout << "State not found: " << endl;
//	    x->print();
//	}
//	 
//    if (c == la.size()) return true;
//    else return false;
//}
//
//shared_ptr<Symbolic_State> get_a_sstate(list<shared_ptr<Symbolic_State> > &li, int index)
//{
//    int i = 0;
//    for (auto x : li) {
//        if (i++ == index)
//            return x;
//    }
//}
//
//TEST_CASE("Simple widened model wieh box", "[symbolic state][box]")
//{
//    Model::reset();
//    ifstream ifs("1t-1p.forts");
//    string str {std::istreambuf_iterator<char>(ifs), 
//	    std::istreambuf_iterator<char>()};
//
//    cout << "------------------ File has been read ----------------------" << endl;
//    cout << str << endl;
//    cout << "------------------------------------------------------------" << endl;
//    build_a_model(str);
//    cout << "------------------ Model has been built --------------------" << endl;
//    MODEL.check_consistency(); // TODO put this inside build_a_model();
//    cout << "------------------ Consistency checked ---------------------" << endl;
//    MODEL.print();
//
//    MODEL.set_sstate_type(BOX_WIDENED);
//    MODEL.SpaceExplorer();
//
//    // build expected states
//
//    auto s_a = build_state({ "idle" }, 
//				     {{"C1",2}, {"D1",8}, {"T1",8}}, 
//				     "c1==0 & p1 >= 0"); 
//    auto s_b = build_state({ "x1R" }, 
//				     {{"C1",2}, {"D1",8}, {"T1",8}}, 
//				     "p1+c1==2 & c1 >=0 & c1 <= 2");
//
//    list<Symbolic_State> expected = { *s_a, *s_b};
//
//
//
//    auto li = MODEL.get_all_states();
//    //cout << "The states we got : " << endl;
//    for (auto x : li) x->print();
//
//    REQUIRE(s_a->equals(get_a_sstate(li, 0)));
//    REQUIRE(s_b->equals(get_a_sstate(li, 1)));
//
//    auto bw_s_a = dynamic_pointer_cast<TEST_Box_Widened_Symbolic_State>(s_a);
//    auto bw_s_b = dynamic_pointer_cast<TEST_Box_Widened_Symbolic_State>(s_b);
//
//    REQUIRE (bw_s_a->equals(dynamic_pointer_cast<Box_Widened_Symbolic_State>(get_a_sstate(li, 0))));
//    REQUIRE (bw_s_b->equals(dynamic_pointer_cast<Box_Widened_Symbolic_State>(get_a_sstate(li, 1))));
//
//    REQUIRE (bw_s_a->equals(dynamic_pointer_cast<Box_Widened_Symbolic_State>(get_a_sstate(li, 0))));
//    REQUIRE (bw_s_b->equals(dynamic_pointer_cast<Box_Widened_Symbolic_State>(get_a_sstate(li, 1))));
//
//    TBox box0(2), box1(2);
//    namespace PPL = Parma_Polyhedra_Library;
//    box0.add_constraint(PPL::Variable(0)<=0);
//
//    box1.add_constraint(PPL::Variable(0)<=2);
//    box1.add_constraint(PPL::Variable(1)<=2);
//
//    REQUIRE( bw_s_a->get_box() == box0);
//    REQUIRE( bw_s_b->get_box() == box1);
//
//}
TEST_CASE("BTest basic operations in box", "[polyhedron][box]")
{
  NNC_Polyhedron poly(2);
  Variable x(0), y(1);
  poly.add_constraint(x-2*y >= -2);
  poly.add_constraint(2*x-y <= 2);
  //poly.add_constraint(x+y <= 1);

  cout << "poly " << poly << endl;

  TBox tpoly = TBox(poly, ANY_COMPLEXITY);
  cout << "Tpoly : " << tpoly << endl;
  Constraint_System  css = tpoly.constraints();
  
  NNC_Polyhedron wpoly(poly.space_dimension());
  for ( auto it = css.begin(); it != css.end(); it++) {
    cout << *it << endl;
    for ( int i = 0; i < css.space_dimension(); i++) {
      if ( it->coefficient(Variable(i)) < 0) wpoly.add_constraint(*it);
    }
  }


  css = poly.constraints();
  for ( auto it = css.begin(); it != css.end(); it++) {
    bool all_neg = true;
    for ( int i = 0; i < css.space_dimension(); i++) {
      if ( it->coefficient(Variable(i)) > 0) {
        all_neg = false;
        break;
      }
    }
    if ( all_neg) wpoly.add_constraint(*it);
  }

  cout << "wpoly : " << wpoly << endl;
}
