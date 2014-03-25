#include "catch.hpp"
#include <constraint_parser.hpp>
#include <assignment_parser.hpp>
#include <automaton_parser.hpp>
#include <string>
#include <fstream>
#include <combined_edge.hpp>
using namespace Parma_Polyhedra_Library::IO_Operators;
using namespace std;



TEST_CASE("Test the combination operation of edges", "[Combined_Edge][combine]")
{

    SECTION("Combine two edges that cannot synchronize") {
      string s1 = "when true sync r1  do {} goto x1R;";
      string s2 = "when true sync r2  do {} goto x2R;";

      edge e1 = build_an_edge(s1);
      edge e2 = build_an_edge(s2);
      vector<string> synclabs1 = {"r1", "r3"};
      vector<string> synclabs2 = {"r2", "r3"};
      Combined_edge ce(e1, "r1", synclabs1);

      vector<Combined_edge> egroups = ce.combine(e2, synclabs2);

      REQUIRE(egroups.size() == 2);
      vector<string> union_labs;
      set_union(synclabs1.begin(), synclabs1.end(), synclabs2.begin(), synclabs2.end(), back_inserter(union_labs));
      Combined_edge ce1(e1, "r1", union_labs);
      Combined_edge ce2(e2, "r2", union_labs);

      REQUIRE(ce1 == egroups.at(0));
      REQUIRE(ce2 == egroups.at(1));

    }

    SECTION("Combine two edges that can synchronize") {
      string s1 = "when true sync r1  do {} goto wait_for_period1;";
      string s2 = "when true sync r1  do {} goto active;";

      edge e1 = build_an_edge(s1);
      edge e2 = build_an_edge(s2);
      vector<string> synclabs1 = {"r1"};
      vector<string> synclabs2 = {"r1", "r2", "r3"};
      Combined_edge ce(e1, "r1", synclabs1);

      vector<Combined_edge> egroups = ce.combine(e2, synclabs2);

      REQUIRE(egroups.size() == 1);

      vector<string> union_labs;
      set_union(synclabs1.begin(), synclabs1.end(), synclabs2.begin(), synclabs2.end(), back_inserter(union_labs));
      Combined_edge ce1(e1, "r1", union_labs);
      vector<edge> &edges = ce1.get_edges();
      edges.push_back(e2);

      REQUIRE(ce1 == egroups.at(0));

    }

    SECTION("Combine an edge with empty sync lab \"\"") {
      string s1 = "when c1==0 do {} goto x2R3W;";
      string s2 = "when true   sync frozen do {} goto active;";

      edge e1 = build_an_edge(s1);
      edge e2 = build_an_edge(s2);
      vector<string> synclabs1 = {"frozen", "r1", "r2", "r3"};
      vector<string> synclabs2 = {"frozen", "r1", "r2", "r3"};
      Combined_edge ce(e1, "", synclabs1);

      vector<Combined_edge> egroups = ce.combine(e2, synclabs2);

      REQUIRE(egroups.size() == 1);

      vector<string> union_labs;
      set_union(synclabs1.begin(), synclabs1.end(), synclabs2.begin(), synclabs2.end(), back_inserter(union_labs));
      Combined_edge ce1(e1, "", union_labs);

      REQUIRE(ce1 == egroups.at(0));

    }

}
