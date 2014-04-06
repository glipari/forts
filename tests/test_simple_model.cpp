#include "catch.hpp" 

#include <fstream>
#include <iostream>
#include <string>
#include <model.hpp>
#include <model_parser.hpp>

using namespace Parma_Polyhedra_Library::IO_Operators;
using namespace std;

TEST_CASE("Simple model", "[model][Space]")
{
    Model::reset();
    ifstream ifs("sm.forts");
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

    
}
