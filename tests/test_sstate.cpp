#include "catch.hpp"
#include <constraint_parser.hpp>
#include <assignment_parser.hpp>
#include <automaton_parser.hpp>
#include <string>
#include <fstream>

using namespace Parma_Polyhedra_Library::IO_Operators;
using namespace std;


TEST_CASE("Test transform the flow specification in a location", 
	  "[location][flow][cvx]")
{
    SECTION("First simple test on transforming rates to a Linear_Constraint") {
	string input = "loc loc0 : while A>=10*B & C <=x*3+2 wait {A== 0& B==1} \n when B==10 do {B'=0} goto loc2; \n when B>=10 do {A'=0} goto loc1;";
	Location l = build_a_location(input);
        cout << "we got the location " << endl;

        auto it = l.get_invariant();
        Variable A(0), B(1);
        Constraint_System css;
        css.insert(A == 0);
        css.insert(B == 1);
        NNC_Polyhedron cvx (css);
        VariableList cvl1;
        Valuations dvl1;
        cvl1.insert(cvl1.end(), "A");
        cvl1.insert(cvl1.end(), "B");
        dvl1.insert(make_pair("x",1));
        VariableList lvars = cvl1;
        NNC_Polyhedron cvx_(cvl1.size());
        cvx_.add_constraints(l.get_flow().to_Linear_Constraint(cvl1, dvl1));
        //cvx_.add_constraints(l.rates_to_Linear_Constraint(cvl1, dvl1,lvars));
        //for (auto it = lvars.begin(); it != lvars.end(); it++) {
	//    PPL::Variable v = get_ppl_variable(cvl1, *it);
	//    Linear_Expr le;
	//    le += 1;
	//    AT_Constraint atc = (v==le);
	//    cout << cvx_ << endl;
	//    cout << atc << endl;
	//    cvx_.add_constraint(atc);
        //}
        REQUIRE ( cvx.contains(cvx_));
        REQUIRE ( cvx_.contains(cvx));
    }
}

TEST_CASE("The new flow specification in a location", 
	  "[location][flow][new]")
{
    SECTION("A simple exemple on the flow specification") {
	string input = "loc loc0 : while A>=10*B & C <=x*3+2 wait {A>=0&A<=0&B>=1&B<=1} \n when B==10 do {B'=0} goto loc2; \n when B>=10 do {A'=0} goto loc1;";
        cout << input << endl;
	Location l = build_a_location(input);
        cout << "----location has been built-------------" << endl;

        auto it = l.get_invariant();
        Variable A(0), B(1);
        Constraint_System css;
        css.insert(A == 0);
        css.insert(B == 1);
        NNC_Polyhedron cvx (css);
        VariableList cvl1;
        Valuations dvl1;
        cvl1.insert(cvl1.end(), "A");
        cvl1.insert(cvl1.end(), "B");
        dvl1.insert(make_pair("x",1));
        VariableList lvars = cvl1;
        NNC_Polyhedron cvx_(cvl1.size());
        //cvx_.add_constraints(l.flow_to_Linear_Constraint(cvl1, dvl1,lvars));
        //for (auto it = lvars.begin(); it != lvars.end(); it++) {
	//    PPL::Variable v = get_ppl_variable(cvl1, *it);
	//    Linear_Expr le;
	//    le += 1;
	//    AT_Constraint atc = (v==le);
	//    cout << cvx_ << endl;
	//    cout << atc << endl;
	//    cvx_.add_constraint(atc);
        //}
        cout << "haha" << endl;
        l.get_flow().print();
        cout << "haha" << endl;
        cvx_.add_constraints(l.get_flow().to_Linear_Constraint(cvl1, dvl1));
        REQUIRE ( cvx.contains(cvx_));
        REQUIRE ( cvx_.contains(cvx));
        REQUIRE ( l.flow_is_using());
        l.get_flow().print();
    }
}

