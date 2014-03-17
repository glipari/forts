#include "catch.hpp"
#include <constraint_parser.hpp>
#include <assignment_parser.hpp>

using namespace Parma_Polyhedra_Library::IO_Operators;
using namespace std;

TEST_CASE("Test the syntax tree for a constraint", 
	  "[TestConstraintTree]")
{
    SECTION("Constraint 1") {
	try {
	    string input = "3+2==14";
	    cout << "1" << endl;
	    auto at_tree = build_a_constraint_tree(input);
	    cout << "2" << endl;
	    DVList dvl;
	    REQUIRE (at_tree->eval(dvl) == false);
	    cout << "3" << endl;
	} catch (std::exception &exc) {
	    cout << "A standard exception" << endl;
	}
    }
    SECTION("Constraint 2") {
	try {
	    string input2 = "3+x-(2+4)==3-(2+4) & 1 <= 2";
	    cout << "4" << endl;
	    auto at_tree2 = build_a_constraint_tree(input2);
	    cout << "5" << endl;
	    DVList dvl2;
	    dvl2.push_back(variable("x",0));
	    cout << "6" << endl;
	    REQUIRE (at_tree2->eval(dvl2) == true);
	} catch (std::exception &exc) {
	    cout << "A standard exception" << endl;
	}   
    } 
}


TEST_CASE("Test parsing assignment", "[assignment][parser]")
{
    SECTION("First simple test") {
	string input = "x' = x + 1";
	assignment a = build_assignment(input);
	REQUIRE(a.x == "x");
	CVList cvl;
	cvl.push_back(variable("x", 0));
	REQUIRE(a.expr->eval(cvl) == 1);
    }
    SECTION("An error is raised") {
	string input = "y = x + z";
	assignment a;
	CHECK_THROWS(a = build_assignment(input));	
    }
}

TEST_CASE("Test the syntax tree for constraint \"true\"", 
	  "[true][TestConstraintTree]")
{
  string input = "true";
  auto at_tree = build_a_constraint_tree(input);
  DVList dvl;
  REQUIRE (at_tree->eval(dvl) == true);
  at_tree->print();

  string input2 = "3+x-(2+4)==3-(2+4) & -x <= 2 & true";
  auto at_tree2 = build_a_constraint_tree(input2);
  DVList dvl2;
  dvl2.push_back(variable("x",0));
  REQUIRE (at_tree2->eval(dvl2) == true);
}
