#include "catch.hpp"
#include <syntax_trees.hpp>

using namespace Parma_Polyhedra_Library::IO_Operators;

TEST_CASE("Test the conversion from constraint to PPL::Constraint_System.", 
	  "[TestConstraintToPPL]")
{
  string input = "x==y & x - (0+ z) >= 0";
  auto at_tree = build_a_constraint_tree(input);
  CVList_ptr cvlp = make_shared<CVList>();
  cvlp->push_back(variable(string("x")));
  cvlp->push_back(variable(string("y")));
  cvlp->push_back(variable(string("z")));
  DVList_ptr vtp = make_shared<DVList>();
  vtp = nullptr;
  Linear_Constraint lc = at_tree->to_Linear_Constraint(cvlp, vtp);

  Variable x(0), y(1), z(2);
  Constraint_System css;
  css.insert(x==y);
  css.insert(x-z>=0);

  C_Polyhedron poly1(css);
  C_Polyhedron poly2(lc);

  REQUIRE ( poly1.contains(poly2) == true);
  REQUIRE ( poly2.contains(poly1) == true);

  string input2 = "x + 2 - (3-y) == 0";
  auto at_tree2 = build_a_constraint_tree(input2);
  CVList_ptr cvlp2 = make_shared<CVList>();
  cvlp2->push_back(variable(string("x")));
  cvlp2->push_back(variable(string("y")));
  Linear_Constraint lc2 = at_tree2->to_Linear_Constraint(cvlp2);

  Constraint_System css2;
  css2.insert(x+y-1==0);

  C_Polyhedron poly_1(css2);
  C_Polyhedron poly_2(lc2);
  REQUIRE ( poly_1.contains(poly_2) == true);
  REQUIRE ( poly_2.contains(poly_1) == true);
}

