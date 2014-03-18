#include "catch.hpp"
#include <constraint_parser.hpp>
#include <assignment_parser.hpp>
#include <automaton_parser.hpp>
#include <string>
#include <fstream>

using namespace Parma_Polyhedra_Library::IO_Operators;
using namespace std;


TEST_CASE("Test transform the rates in a location to a cvx", "[location][rates][cvx]")
{
    SECTION("First simple test on transforming rates to a Linear_Constraint") {
	string input = "loc loc0 : while A>=10*B & C <=x*3+2 wait {A' = 0, B'=1} \n when B==10 do {B'=0} goto loc2; \n when B>=10 do {A'=0} goto loc1;";
	location l = build_a_location(input);

        auto it = l.invariant;
        Variable A(0), B(1), C(2);
        Constraint_System css;
        css.insert(A == 0);
        css.insert(B == 1);
        css.insert(C == 1);
        C_Polyhedron cvx (css);
        CVList cvl1;
        DVList dvl1;
        cvl1.push_back(variable("A"));
        cvl1.push_back(variable("B"));
        cvl1.push_back(variable("C"));
        dvl1.push_back(variable("x",1));
        C_Polyhedron cvx_(l.rates_to_Linear_Constraint(cvl1, dvl1));
        REQUIRE ( cvx.contains(cvx_));
        REQUIRE ( cvx_.contains(cvx));
        
    }
}

