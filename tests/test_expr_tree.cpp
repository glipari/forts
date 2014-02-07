#include "catch.hpp"
#include <syntax_trees.hpp>

TEST_CASE("Test the syntax tree for an expression", "[TestExprTree]")
{
  string input = "3+2-(2+4)";
  auto expr_tree = build_an_expr_tree(input);
  REQUIRE (expr_tree->compute() == -1);

  string input2 = "3+x-(2+4)";
  auto expr_tree2 = build_an_expr_tree(input2);
  REQUIRE (expr_tree2->compute() == -3);
}

