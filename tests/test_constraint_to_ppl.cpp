#include "catch.hpp"
#include <constraint_parser.hpp>
#include <ppl_adapt.hpp>

using namespace Parma_Polyhedra_Library::IO_Operators;
using namespace std;

TEST_CASE("Test the conversion from constraint to PPL::Constraint_System.", 
	  "[TestConstraintToPPL]")
{
  string input1st = "2*x==y & x - (0+ z) >= 0 & c*y<=a*z - b";
  auto at_tree1st = build_a_constraint_tree(input1st);
  VariableList cvl1st;
  Valuations dvl1st;
  dvl1st.insert(make_pair("a",4));
  dvl1st.insert(make_pair("b",2));
  dvl1st.insert(make_pair("c",3));
  cvl1st.insert(cvl1st.end(), "x");
  cvl1st.insert(cvl1st.end(), "y");
  cvl1st.insert(cvl1st.end(), "z");
  Linear_Constraint lc1st = at_tree1st.to_Linear_Constraint(cvl1st, dvl1st);

  cout << "lc1st: " << lc1st << endl;

  PPL::Variable x(0), y(1), z(2);
  Constraint_System css1st;
  css1st.insert(2*x==y);
  css1st.insert(x-z>=0);
  css1st.insert(3*y<=4*z-2);

  C_Polyhedron poly_css1st(css1st);
  C_Polyhedron poly_lc1st(lc1st);

  REQUIRE ( poly_css1st.contains(poly_lc1st) == true);
  REQUIRE ( poly_lc1st.contains(poly_css1st) == true);

  string input2nd = "2*x + 2*1 - (3-wcet1) == 0 & 2*wcet1<= dline1";
  auto at_tree2nd = build_a_constraint_tree(input2nd);
  VariableList cvl2nd;
  Valuations dvl2nd;
  dvl2nd.insert(make_pair("dline1",10));
  cvl2nd.insert(cvl2nd.end(), "wcet1");
  cvl2nd.insert(cvl2nd.end(), "x");
  Linear_Constraint lc2nd = at_tree2nd.to_Linear_Constraint(cvl2nd, dvl2nd);

  cout << "lc2nd: " << lc2nd << endl;

  Constraint_System css2nd;
  css2nd.insert(2*y+x-1==0);
  css2nd.insert(2*x <= 10);

  cout << "css2nd: " << css2nd << endl;

  C_Polyhedron poly_css2nd(css2nd);
  C_Polyhedron poly_lc2nd(lc2nd);
  REQUIRE ( poly_css2nd.contains(poly_lc2nd) == true);
  REQUIRE ( poly_lc2nd.contains(poly_css2nd) == true);

}

