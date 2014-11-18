#include "catch.hpp"
#include <constraint_parser.hpp>
#include <assignment_parser.hpp>
#include <automaton_parser.hpp>
#include <string>
#include <fstream>

using namespace Parma_Polyhedra_Library::IO_Operators;
using namespace std;


TEST_CASE("Test parsing an edge", "[edge][parser]")
{
    SECTION("First simple test") {
	string input = "when A>=10*B & C <=x*3+2 do {x' = x + 1, y'=y+10} goto loc1;";
	Edge e = build_an_edge(input);
        REQUIRE(e.get_dest() == "loc1");
        Assignment a = e.get_assignment_at(0);
        Assignment b = e.get_assignment_at(1);
	REQUIRE(a.get_var() == "x");
	REQUIRE(b.get_var() == "y");
	Valuations cvl;
	cvl.insert(make_pair("x", 0));
	cvl.insert(make_pair("y", 1));
	REQUIRE(a.eval(cvl) == 1);
	REQUIRE(b.eval(cvl) == 11);

        auto g = e.get_guard();
	g.print();
        cout << endl;
        Variable A(0), B(1), C(2);
        Constraint_System css;
        css.insert(A >= 10*B);
        css.insert(C <= 5);
        NNC_Polyhedron cvx (css);
        VariableList cvl1;
        Valuations dvl1;
        cvl1.insert(cvl1.end(), "A");
        cvl1.insert(cvl1.end(), "B");
        cvl1.insert(cvl1.end(), "C");
        dvl1.insert(make_pair("x",1));
        cout << cvx << endl;
        Linear_Constraint lc = e.guard_to_Linear_Constraint(cvl1, dvl1); 
        NNC_Polyhedron poly(lc);
        REQUIRE ( cvx.contains(poly));
        REQUIRE ( poly.contains(cvx));

    }
    SECTION("An error is raised") {
	string input = "y = x + z";
	//Assignment a;
	CHECK_THROWS(Assignment a = build_assignment(input));	
    }
}

TEST_CASE("Test parsing a location", "[location][parser]")
{
    SECTION("First simple test on location") {
	string input = "loc loc0 : while A>=10*B & C <=x*3+2 wait {A==0& B==1} \n when B==10 do {B'=0} goto loc2; \n when B>=10 do {A'=0} goto loc1;";
        //cout << "xxxxx" << endl;
	Location l = build_a_location(input);
        //l.print();

        REQUIRE(l.get_name() == "loc0");
        //Assignment a = l.get_rates().at(0);
        //cout << a.x << endl;
        //Assignment b = l.get_rates().at(1);
        //cout << b.x << endl;
        constraint_node flow = l.get_flow();
	REQUIRE(flow.has_variable("A"));
	REQUIRE(flow.has_variable("B"));
	//REQUIRE(a.get_var() == "A");
	//REQUIRE(b.get_var() == "B");
	//Valuations cvl;
	//REQUIRE(a.eval(cvl) == 0);
	//REQUIRE(b.eval(cvl) == 1);

        auto it = l.get_invariant();
        Variable A(0), B(1), C(2);
        Constraint_System css;
        css.insert(A >= 10*B);
        css.insert(C <= 5);
        NNC_Polyhedron cvx (css);
        VariableList cvl1;
        Valuations dvl1;
        cvl1.insert(cvl1.end(), "A");
        cvl1.insert(cvl1.end(), "B");
        cvl1.insert(cvl1.end(), "C");
        dvl1.insert(make_pair("x",1));
        cout << cvx << endl;
        Linear_Constraint lc = it.to_Linear_Constraint(cvl1, dvl1); 
        NNC_Polyhedron poly(lc);
        REQUIRE ( cvx.contains(poly));
        REQUIRE ( poly.contains(cvx));

        Edge e0 = l.get_edges().at(0);
        REQUIRE ( e0.get_dest() == "loc2");
        Assignment ass0 =  e0.get_assignment_at(0);
        REQUIRE ( ass0.eval(dvl1) == 0);
        REQUIRE ( ass0.get_var() == "B");

        auto guard = e0.get_guard();
        Constraint_System g_css0;
        g_css0.insert( B==10);
        NNC_Polyhedron g_cvx0(g_css0);
        NNC_Polyhedron g_poly0(e0.guard_to_Linear_Constraint(cvl1,dvl1));
        REQUIRE ( g_cvx0.contains(g_poly0));
        REQUIRE ( g_poly0.contains(g_cvx0));

        Edge e1 = l.get_edges().at(1);
        REQUIRE ( e1.get_dest() == "loc1");

        ass0 =  e1.get_assignment_at(0);
        REQUIRE ( ass0.eval(dvl1) == 0);
        REQUIRE ( ass0.get_var() == "A");
        
        guard = e1.get_guard();
        Constraint_System g_css;
        g_css.insert( B>=10);
        NNC_Polyhedron g_cvx(g_css);
        NNC_Polyhedron g_poly(guard.to_Linear_Constraint(cvl1,dvl1));
        cout << "g poly " << g_poly << endl;
        cout << "g cvx " << g_cvx << endl;
        REQUIRE ( g_cvx.contains(g_poly));
        REQUIRE ( g_poly.contains(g_cvx));
    }
}

TEST_CASE("Test printing a location", "[location][printer]")
{
    SECTION("First simple test on printing a location") {
	string input = "loc loc0 : while A>=10*B & C <=x*3+2 wait {A== 0& B==1} \n when B==10+A do {B'=0, B'=100} goto loc2; \n when B>=100*C do {A'=0, B'=2} goto loc1;\n when B>=100*C do {A'=0, B'=2} goto loc1;";
	Location l = build_a_location(input);
        l.print();
        cout << " -------------------- " << endl;
	string input2 = "loc loc0 : while A>=10*B & C <=x*3+2 wait {} \n when B==10+A do {B'=0, B'=100} goto loc2; \n when B>=100*C do {A'=0, B'=2} goto loc1;\n when B>=100*C do {A'=0, B'=2} goto loc1;";
	Location l2 = build_a_location(input2);
        l2.print();
    }
}

TEST_CASE("Test printing an automaton", "[automaton][printer]")
{
    SECTION("To print the location inside an automaton first.") {
	string input1 = "loc loc0 : while A>=10*B & C <=x*3+2 wait {A== 0& B==1} \n when B==10+A do {B'=0, B'=100} goto loc2; \n when B>=100*C do {A'=0, B'=2} goto loc1;\n when B>=100*C do {A'=0, B'=2} goto loc3;\n"; 
        Location loc1 = build_a_location(input1);
        loc1.print();
    }

    SECTION("First simple test on printing an automaton") {
	string input1 = "automaton A \n sync : a1, a2; \n loc loc0 : while A>=10*B & C <=x*3+2 wait {A== 0& A==0&B==1} \n when B==10+A do {B'=0, B'=100} goto loc2; \n when B>=100*C do {A'=0, B'=2} goto loc1;\n when B>=100*C do {A'=0, B'=2} goto loc3;  \n loc loc1 : while A>=10*B & C <=x*3+2 wait {A== 0& B==1} \n when B==10+A do {B'=0, B'=100} goto loc2; \n when B>=100*C do {A'=0, B'=2} goto loc5;\n when B>=100*C do {A'=0, B'=2} goto loc4;  end";
        automaton aton1 = build_an_automaton(input1);
        aton1.print();
    }

    SECTION("Read an automaton from a file") {
	std::ifstream ifs1("automaton1.txt");
        std::string str1((std::istreambuf_iterator<char>(ifs1)), std::istreambuf_iterator<char>()); 
        std::cout << str1 << endl;
        //str1.erase(std::remove(str1.begin(), str1.end(), '\n'), str1.end());
        for (auto it = str1.begin(); it != str1.end(); it++)
          if (*it == '\n')
            *it = ' ';
        automaton aton1 = build_an_automaton(str1);
        aton1.print();
    }
}
