#include "catch.hpp"
#include <ppl_adapt.hpp>

using namespace std;
using namespace Parma_Polyhedra_Library::IO_Operators;

TEST_CASE("Testing ppl coefficients", "[ppl]")
{
    PPL::Variable x(0);
    Constraint c ( (2 * x < 1) );
    PPL::Linear_Expression e;
    e += c.coefficient(x) * x; 
    cout << "Constraint: " << c << endl;
    cout << "Linear Expression: " << e << endl;
}
