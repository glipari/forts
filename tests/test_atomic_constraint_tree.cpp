#include "catch.hpp"
#include <syntax_trees.hpp>

TEST_CASE("Test the syntax tree for an atomic constraint", "[TestAtomicConstraintTree]")
{
  string input = "3+x-(2+4)==4";
  //string input = "3+2>4";
  //string input = "3+1<=4+2";
  auto at_tree = build_an_at_tree(input);
  REQUIRE (at_tree->is_satisfied() == false);

  //string input2 = "3+x-(2+4)==3-(2+4)";
  //auto at_tree2 = build_an_at_tree(input2);
  //REQUIRE (at_tree2->is_satisfied() == true);
}

