#include "catch.hpp"
#include <syntax_trees.hpp>

using namespace Parma_Polyhedra_Library::IO_Operators;

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

