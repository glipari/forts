#include "catch.hpp"
#include <constraint_parser.hpp>
#include <assignment_parser.hpp>
#include <automaton_parser.hpp>
#include <string>
#include <fstream>
#include <model_parser.hpp>
#include <model.hpp>
using namespace Parma_Polyhedra_Library::IO_Operators;
using namespace std;



TEST_CASE("Parsing of parameter list in a model", "[model][parameters]")
{

    SECTION("Read a model and obtain the parameter list") {
	Model::reset();
	std::ifstream ifs1("param-model.forts");
        std::string str1((std::istreambuf_iterator<char>(ifs1)), std::istreambuf_iterator<char>()); 
        MODEL.reset();
        build_a_model(str1);
        MODEL.check_consistency();
        const vector<Parameter> &parameters = MODEL.get_parameters();
        //Parameter param1("P3", 10, 100);
        //Parameter param2("P2", 1, 9);
        //Parameter param3("P1", 0, 10);
        vector<Parameter> expected = { {"P1", 0, 10}, {"P2", 1, 9}, {"P3", 10, 100}};
        for ( int i = 0; i < 3; i++) {
            REQUIRE (parameters[i].min == expected[i].min);
            REQUIRE (parameters[i].max == expected[i].max);
            REQUIRE (parameters[i].name == expected[i].name);
        }
        //MODEL.print();
    }

    //SECTION("A parameter must be specified in the form of lhs<=name<=rhs, otherwise an error would be raised.") {
	//    Model::reset();
	//    std::ifstream ifs1("error-param-model.forts");
    //    std::string str1((std::istreambuf_iterator<char>(ifs1)), std::istreambuf_iterator<char>()); 
    //    MODEL.reset();
    //    build_a_model(str1);
    //    CHECK_THROWS(build_a_model(str1));
    //}

}
