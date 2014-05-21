#include "catch.hpp" 

#include <fstream>
#include <iostream>
#include <string>
#include <model.hpp>
#include <model_parser.hpp>
#include <constraint_parser.hpp>
#include <time_abstract_state.hpp>

using namespace Parma_Polyhedra_Library::IO_Operators;
using namespace std;
using namespace tipa;

TEST_CASE("Tests on Time Abstract State", "[Time_Abstract_State][trace]")
{
    SECTION ("Build and backtrack the trace of a Time Abstract State") {
        Model::reset();
        ifstream ifs("time-abstract-state.forts");
        string str {std::istreambuf_iterator<char>(ifs), 
	        std::istreambuf_iterator<char>()};

        cout << "------------------ File has been read ----------------------" << endl;
        cout << str << endl;
        cout << "------------------------------------------------------------" << endl;
        build_a_model(str);
        cout << "------------------ Model has been built --------------------" << endl;
        MODEL.check_consistency(); // TODO put this inside build_a_model();
        cout << "------------------ Consistency checked ---------------------" << endl;
        MODEL.print();

        MODEL.SpaceExplorer();



        auto li = MODEL.get_all_states();
        auto it = li.begin();
        auto ss1 = *(it++);
        auto ss2 = *(it++);
        auto ss3 = *(it++);

        Time_Abstract_State tas(ss2);
        const vector<Combined_edge> & tr = tas.get_trace();

        const vector<Location *> &locations = tr[0].get_locations();
        REQUIRE( locations[0] == (ss1->get_locations())[0]);
        REQUIRE( locations[1] == (ss1->get_locations())[1]);
        REQUIRE( locations[0]->get_name() == "A");
        REQUIRE( locations[1]->get_name() == "D");
        const vector<Edge> & edges = tr[0].get_edges();
        REQUIRE( edges.size() == 1);
        REQUIRE(edges[0].get_index() == ((ss1->get_locations())[0]->get_edges())[0].get_index());




        //REQUIRE (s_a->equals(s1));
        //REQUIRE (s_b->equals(s2));
        //REQUIRE (s_c->equals(s3));
    }

}
