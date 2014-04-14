#include "catch.hpp"
#include <constraint_parser.hpp>
#include <assignment_parser.hpp>
#include <automaton_parser.hpp>
#include <string>
#include <fstream>
#include <sstate.hpp>

using namespace Parma_Polyhedra_Library::IO_Operators;
using namespace std;


TEST_CASE("Test the signature of a symbolic state", 
	  "[signature]")
{
    SECTION("First simple test to read a signature") {
        Signature sig1("x1R2R3W");
        Signature sig2("x1R3R");
        Signature sig3("idle");
        Signature sig4("x1R2R3W");
        Signature sig5("x1R");


        REQUIRE (sig1.includes(sig2));
        REQUIRE (sig2.includes(sig3));
        REQUIRE (sig1.includes(sig3));
        REQUIRE (sig1.includes(sig3));
        REQUIRE (sig5.includes(sig3));
        REQUIRE (sig4.includes(sig3));

        REQUIRE (sig1.get_active_tasks() == 14);
        REQUIRE (sig2.get_active_tasks() == 10);
        REQUIRE (sig3.get_active_tasks() == 0);
        REQUIRE (sig5.get_active_tasks() == 2);
        REQUIRE (sig1 == sig4);
    }
}

