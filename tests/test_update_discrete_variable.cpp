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
      //cvars.insert("D");

      dvars.insert(make_pair("r1",10));
      dvars.insert(make_pair("r2",200));

      PPL::Variables_Set vs = e1.get_assignment_vars(cvars);

      PPL::Variable A(0), B(1), C(2), D(3);
      PPL::Constraint_System css;
      css.insert(A+B<= 1000);
      css.insert(C+B<= 1000);
      css.insert(C<= 50);
      css.insert(10<=C);
      //css.insert(D==2);

      C_Polyhedron cvx(css);
      cvx.unconstrain(vs);

      C_Polyhedron ass_cvx(cvars.size()*2);
      ass_cvx.add_constraints(e1.ass_to_Linear_Constraint(cvars,dvars));
      /** Remove the lower space dimensions from ass_cvx. */
      PPL::Variables_Set lower_dims;
      for ( unsigned i = 0; i < cvars.size(); i++)
        lower_dims.insert(PPL::Variable(i));
      ass_cvx.remove_space_dimensions(lower_dims);

      cvx.intersection_assign(ass_cvx);

      Constraint_System css_;
      css_.insert(A==10);
      css_.insert(B==201);
      css_.insert(C<= 50);
      css_.insert(10<=C);
      //css_.insert(D==12);

      C_Polyhedron cvx_(css_);


      REQUIRE(cvx.contains(cvx_));
      REQUIRE(cvx_.contains(cvx));

      REQUIRE(dvars["r1"]==0);
      REQUIRE(dvars["r2"]==201);

    }

    SECTION("A continuous variable is updated by itself.") {
      string s1 = "when true do {A'=A+1, r1'=r1+r2, B'=B+r2} goto x1R;";

      Edge e1 = build_an_edge(s1);

      VariableList cvars;
      Valuations dvars;

      cvars.insert("A");
      cvars.insert("B");
      cvars.insert("C");

      dvars.insert(make_pair("r1",10));
      dvars.insert(make_pair("r2",200));

      PPL::Variables_Set vs = e1.get_assignment_vars(cvars);

      PPL::Variable A(0), B(1), C(2), D(3);
      PPL::Constraint_System css;
      css.insert(A==2);
      css.insert(B>=30);
      css.insert(B<=70);

      C_Polyhedron cvx(cvars.size());
      cvx.add_constraints(css);

      C_Polyhedron ass_cvx(cvars.size()*2);
      ass_cvx.add_constraints(e1.ass_to_Linear_Constraint(cvars,dvars));
      
      ass_cvx.add_constraints(cvx.constraints());
      //cout << "-1 : " << ass_cvx << endl;
      //cvx.add_space_dimensions_and_embed(cvars.size());
      //ass_cvx.intersection_assign(cvx);
      //cout << "-2 : " << ass_cvx << endl;

      /** Remove the lower space dimensions from ass_cvx. */
      PPL::Variables_Set lower_dims;
      for ( unsigned i = 0; i < cvars.size(); i++)
        lower_dims.insert(PPL::Variable(i));
      ass_cvx.remove_space_dimensions(lower_dims);

      //cvx.remove_higher_space_dimensions(cvars.size());
      cvx.unconstrain(vs);
      cvx.intersection_assign(ass_cvx);

      Constraint_System css_;
      css_.insert(A==3);
      css_.insert(B>=230);
      css_.insert(B<=270);
      //css_.insert(B+10==C);

      C_Polyhedron cvx_(cvars.size());
      cvx_.add_constraints(css_);

      //cout << "-2 : " << cvx << endl;

      REQUIRE(cvx.contains(cvx_));
      REQUIRE(cvx_.contains(cvx));
      REQUIRE(dvars["r1"]==210);


    }

}
