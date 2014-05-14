#include "catch.hpp"
#include <constraint_parser.hpp>
#include <assignment_parser.hpp>
#include <automaton_parser.hpp>
#include <model_parser.hpp>
#include <string>
#include <fstream>
#include <combined_edge.hpp>
using namespace Parma_Polyhedra_Library::IO_Operators;
namespace PPL = Parma_Polyhedra_Library;
using namespace std;



TEST_CASE("Test the generated sumbolic states in Space", "[][]")
{

    SECTION("Check every symbolic state in Space") {

        MODEL.reset();
        /**
         * 2t.mod : two tasks are scheduled by preemptive
         * FPFP on a single processor.
         */
        std::ifstream ifs("2t.mod");
        std::string str((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
        build_a_model(str);
        /** This is a must check. */
        MODEL.check_consistency();

        MODEL.print();

        auto init = MODEL.init_sstate();

        automaton &act1 = MODEL.get_automaton_by_name("act1");
        act1.print();
        automaton &act2 = MODEL.get_automaton_by_name("act2");
        act2.print();
        automaton &sched = MODEL.get_automaton_by_name("sched");
        sched.print();

        PPL::Variable p1(0), c1(1), d1(2), p2(3), c2(4), d2(5);
        int O1 = 0, T1 = 2, C1 = 1, D1 = 2, O2 = 1, T2 = 4, C2 = 2, D2 = 4;


        list<shared_ptr<Symbolic_State> > sstates, Space;
        auto curr = init;

        Valuations dvars;
        dvars.insert(make_pair("O1",0));
        dvars.insert(make_pair("T1",2));
        dvars.insert(make_pair("C1",1));
        dvars.insert(make_pair("D1",2));
        dvars.insert(make_pair("O2",1));
        dvars.insert(make_pair("T2",4));
        dvars.insert(make_pair("C2",2));
        dvars.insert(make_pair("D2",4));

        vector<string> loc_names;
        NNC_Polyhedron cvx(6);

        // The sstate 0
        loc_names.push_back("wait_for_offset1");
        loc_names.push_back("wait_for_offset2");
        loc_names.push_back("idle");
        cvx.add_constraint(p1==0);
        cvx.add_constraint(p2==0);
        cvx.add_constraint(c1==0);
        cvx.add_constraint(c2==0);
        cvx.add_constraint(d1==0);
        cvx.add_constraint(d2==0);

        auto ss0 = make_shared<Symbolic_State>(loc_names, dvars, cvx);

        REQUIRE(ss0->equals(curr));
        sstates.push_back(ss0);
        Space.push_back(curr);

        // reset 
        loc_names.clear();
        cvx = NNC_Polyhedron(6);
        
        //the sstate 1
        vector<shared_ptr<Symbolic_State> > nsss = MODEL.Post(curr);
        auto it = nsss.begin();
        while( it != nsss.end()) {
            if ( (*it)->is_empty()) {
                it = nsss.erase(it);
                continue;
            }
            it ++;
        }
        REQUIRE(nsss.size() == 1);
        curr = nsss[0];
        loc_names.push_back("wait_for_period1");
        loc_names.push_back("wait_for_offset2");
        loc_names.push_back("x1R");
        cvx.add_constraint(p1<=1);
        cvx.add_constraint(p1>=0);
        cvx.add_constraint(p2==p1);
        cvx.add_constraint(c1==p1);
        cvx.add_constraint(c2==p1);
        cvx.add_constraint(d1==p1);
        cvx.add_constraint(d2==p1);

        auto ss1 = make_shared<Symbolic_State> (loc_names, dvars, cvx);

        REQUIRE(ss1->equals(curr));
        sstates.push_back(ss1);
        Space.push_back(curr);

        // reset 
        loc_names.clear();
        cvx = NNC_Polyhedron(6);


        


    }

}
