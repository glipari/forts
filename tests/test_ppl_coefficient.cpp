#include "catch.hpp"

#include <syntax_trees.hpp>

using namespace Parma_Polyhedra_Library::IO_Operators;

TEST_CASE("Testing ppl coefficients", "[ppl]")
{
    Variable x(0);
    Constraint c ( (2 * x < 1) );
    Linear_Expression e;
    e += c.coefficient(x) * x; 
    cout << "Constraint: " << c << endl;
    cout << "Linear Expression: " << e << endl;
}
