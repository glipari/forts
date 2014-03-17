#include "catch.hpp"
#include <constraint_parser.hpp>
#include <assignment_parser.hpp>
#include <automaton_parser.hpp>
#include <string>
#include <fstream>
#include <model_parser.hpp>
using namespace Parma_Polyhedra_Library::IO_Operators;
using namespace std;



TEST_CASE("First test on parsing a model", "[model][printer]")
{

    SECTION("Read a model from a file") {
	std::ifstream ifs1("model1.txt");
        std::string str1((std::istreambuf_iterator<char>(ifs1)), std::istreambuf_iterator<char>()); 
        std::cout << str1 << endl;
        for (auto it = str1.begin(); it != str1.end(); it++)
          if (*it == '\n')
            *it = ' ';
        cout << str1 << endl;
        model mod1 = build_a_model(str1);
        mod1.check_consistency();
        mod1.print();
    }
}
