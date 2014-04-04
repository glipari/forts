#include "catch.hpp"
#include <constraint_parser.hpp>
#include <assignment_parser.hpp>
#include <automaton_parser.hpp>
#include <string>
#include <fstream>
#include <combined_edge.hpp>
using namespace Parma_Polyhedra_Library::IO_Operators;
namespace PPL = Parma_Polyhedra_Library;
using namespace std;



TEST_CASE("Test the update of a discrete variable in the edge", "[][]")
{

    SECTION("Update discrete variable in an edge") {
      string s1 = "when true sync r1  do {A'=r1, r1'=0, r2'=r2+1, B'=r2} goto x1R;";

      Edge e1 = build_an_edge(s1);

      VariableList cvars;
      Valuations dvars;

      cvars.insert("A");
      cvars.insert("B");
      cvars.insert("C");

      dvars.insert(make_pair("r1",10));
      dvars.insert(make_pair("r2",200));

      PPL::Variables_Set vs = e1.get_assignment_vars(cvars);

      PPL::Variable A(0), B(1), C(2);
      PPL::Constraint_System css;
      css.insert(A+B<= 1000);
      css.insert(C+B<= 1000);
      css.insert(C<= 50);
      css.insert(10<=C);
      C_Polyhedron cvx(css);
      //Variables_Set vss;
      //vss.insert(A);
      //vss.insert(B);
      cvx.unconstrain(vs);

      cvx.add_constraints(e1.ass_to_Linear_Constraint(cvars,dvars));

      Constraint_System css_;
      css_.insert(A==10);
      css_.insert(B==201);
      css_.insert(C<= 50);
      css_.insert(10<=C);

      C_Polyhedron cvx_(css_);


      REQUIRE(cvx.contains(cvx_));
      REQUIRE(cvx_.contains(cvx));

      REQUIRE(dvars["r1"]==0);
      REQUIRE(dvars["r2"]==201);

    }

}
