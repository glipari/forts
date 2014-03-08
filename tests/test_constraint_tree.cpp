#include "catch.hpp"
#include <syntax_trees.hpp>
#include <assignment.hpp>

using namespace Parma_Polyhedra_Library::IO_Operators;
using namespace std;

TEST_CASE("Test the syntax tree for a constraint", 
	  "[TestConstraintTree]")
{
  string input = "3+2==14";
  auto at_tree = build_a_constraint_tree(input);
  DVList dvl;
  REQUIRE (at_tree->eval(dvl) == false);

  string input2 = "3+x-(2+4)==3-(2+4) & 1 <= 2";
  auto at_tree2 = build_a_constraint_tree(input2);
  DVList dvl2;
  dvl2.push_back(variable("x",0));
  REQUIRE (at_tree2->eval(dvl2) == true);
}


TEST_CASE("Test parsing assignment", "[assignment][parser]")
{
    string input = "x' = x + 1";
    assignment a = build_assignment(input);
    REQUIRE(a.x == "x");
    CVList cvl;
    cvl.push_back(variable("x", 0));
    REQUIRE(a.expr->eval(cvl) == 1);
}
