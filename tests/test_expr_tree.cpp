#include "catch.hpp"
#include <expression_parser.hpp>

using namespace std;

TEST_CASE("Test the syntax tree for an expression", "[TestExprTree]")
{
    Valuations dv;
    string input = "3+2-(2+4)";
    auto expr_tree = build_expression(input);
    REQUIRE (expr_tree->eval(dv) == -1);

    dv.insert(make_pair("x", 0));

    string input2 = "3+x-(2+4)";
    auto expr_tree2 = build_expression(input2);
    REQUIRE (expr_tree2->eval(dv) == -3);

    set_valuation(dv, "x", 1);
    REQUIRE (expr_tree2->eval(dv) == -2);
}

TEST_CASE("Test unary operator in an expression", "[unary][TestExprTree]")
{

    SECTION("First test on the unary operator '-' ") {
      Valuations dv;
      string input = "-3+2-(2+4)";
      auto expr_tree = build_expression(input);
      REQUIRE (expr_tree->eval(dv) == -7);

      //dv.insert(make_pair("x", 0));

      //string input2 = "3+x-(2+4)";
      //auto expr_tree2 = build_expression(input2);
      //REQUIRE (expr_tree2->eval(dv) == -3);

      //change_var_value("x", dv, 1);
      //REQUIRE (expr_tree2->eval(dv) == -2);
    }
}
