#include "catch.hpp"
#include <syntax_trees.hpp>
#include <assignment.hpp>
#include <automaton.hpp>

using namespace Parma_Polyhedra_Library::IO_Operators;
using namespace std;

//TEST_CASE("Test the syntax tree for a constraint", 
//	  "[TestConstraintTree]")
//{
//  string input = "3+2==14";
//  auto at_tree = build_a_constraint_tree(input);
//  DVList dvl;
//  REQUIRE (at_tree->eval(dvl) == false);
//
//  string input2 = "3+x-(2+4)==3-(2+4) & 1 <= 2";
//  auto at_tree2 = build_a_constraint_tree(input2);
//  DVList dvl2;
//  dvl2.push_back(variable("x",0));
//  REQUIRE (at_tree2->eval(dvl2) == true);
//}


TEST_CASE("Test parsing an edge", "[edge][parser]")
{
    SECTION("First simple test") {
	string input = "when A>=10*B & C <=x*3+2 do {x' = x + 1, y'=y+10} goto loc1;";
	edge e = build_an_edge(input);
        REQUIRE(e.dest == "loc1");
        assignment a = e.assignments.at(0);
        assignment b = e.assignments.at(1);
	REQUIRE(a.x == "x");
	REQUIRE(b.x == "y");
	CVList cvl;
	cvl.push_back(variable("x", 0));
	cvl.push_back(variable("y", 1));
	REQUIRE(a.expr->eval(cvl) == 1);
	REQUIRE(b.expr->eval(cvl) == 11);

        auto it = e.guard;
        it->print();
        cout << endl;
        Variable A(0), B(1), C(2);
        Constraint_System css;
        css.insert(A >= 10*B);
        css.insert(C <= 5);
        C_Polyhedron cvx (css);
        CVList cvl1;
        DVList dvl1;
        cvl1.push_back(variable("A"));
        cvl1.push_back(variable("B"));
        cvl1.push_back(variable("C"));
        dvl1.push_back(variable("x",1));
        cout << cvx << endl;
        Linear_Constraint lc = it->to_Linear_Constraint(cvl1, dvl1); 
        C_Polyhedron poly(lc);
        REQUIRE ( cvx.contains(poly));
        REQUIRE ( poly.contains(cvx));

    }
    SECTION("An error is raised") {
	string input = "y = x + z";
	assignment a;
	CHECK_THROWS(a = build_assignment(input));	
    }
}

TEST_CASE("Test parsing a location", "[location][parser]")
{
    SECTION("First simple test on location") {
	string input = "loc loc0 : while A>=10*B & C <=x*3+2 wait {A' = 0, B'=1} \n when B==10 do {B'=0} goto loc2; \n when B>=10 do {A'=0} goto loc1;";
        cout << "xxxxx" << endl;
	location l = build_a_location(input);
        l.print();

        REQUIRE(l.name == "loc0");
        assignment a = l.rates.at(0);
        cout << a.x << endl;
        assignment b = l.rates.at(1);
        cout << b.x << endl;
	REQUIRE(a.x == "A");
	REQUIRE(b.x == "B");
	CVList cvl;
	REQUIRE(a.expr->eval(cvl) == 0);
	REQUIRE(b.expr->eval(cvl) == 1);

        auto it = l.invariant;
        Variable A(0), B(1), C(2);
        Constraint_System css;
        css.insert(A >= 10*B);
        css.insert(C <= 5);
        C_Polyhedron cvx (css);
        CVList cvl1;
        DVList dvl1;
        cvl1.push_back(variable("A"));
        cvl1.push_back(variable("B"));
        cvl1.push_back(variable("C"));
        dvl1.push_back(variable("x",1));
        cout << cvx << endl;
        Linear_Constraint lc = it->to_Linear_Constraint(cvl1, dvl1); 
        C_Polyhedron poly(lc);
        REQUIRE ( cvx.contains(poly));
        REQUIRE ( poly.contains(cvx));


        edge e0 = l.outgoings.at(0);
        REQUIRE ( e0.dest == "loc2");
        assignment ass0 =  e0.assignments.at(0);
        REQUIRE ( ass0.expr->eval(dvl1) == 0);
        REQUIRE ( ass0.x == "B");

        auto guard = e0.guard;
        Constraint_System g_css0;
        g_css0.insert( B==10);
        C_Polyhedron g_cvx0(g_css0);
        C_Polyhedron g_poly0(guard->to_Linear_Constraint(cvl1,dvl1));
        REQUIRE ( g_cvx0.contains(g_poly0));
        REQUIRE ( g_poly0.contains(g_cvx0));

        edge e1 = l.outgoings.at(1);
        REQUIRE ( e1.dest == "loc1");

        ass0 =  e1.assignments.at(0);
        REQUIRE ( ass0.expr->eval(dvl1) == 0);
        REQUIRE ( ass0.x == "A");
        
        guard = e1.guard;
        Constraint_System g_css;
        g_css.insert( B>=10);
        C_Polyhedron g_cvx(g_css);
        C_Polyhedron g_poly(guard->to_Linear_Constraint(cvl1,dvl1));
        cout << "g poly " << g_poly << endl;
        cout << "g cvx " << g_cvx << endl;
        REQUIRE ( g_cvx.contains(g_poly));
        REQUIRE ( g_poly.contains(g_cvx));


    }
}

TEST_CASE("Test printing a location", "[location][printer]")
{
    SECTION("First simple test on printing a location") {
	string input = "loc loc0 : while A>=10*B & C <=x*3+2 wait {A' = 0, B'=1} \n when B==10+A do {B'=0, B'=100} goto loc2; \n when B>=100*C do {A'=0, B'=2} goto loc1;";
	location l = build_a_location(input);
        l.print();
    }
}
