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



TEST_CASE("First test on SpaceExplorer", "[model][Space]")
{

    SECTION("Explore the sstate space") {
	std::ifstream ifs1("test.imi");
        std::string str1((std::istreambuf_iterator<char>(ifs1)), std::istreambuf_iterator<char>()); 
        std::cout << str1 << endl;
        for (auto it = str1.begin(); it != str1.end(); it++)
          if (*it == '\n')
            *it = ' ';
        cout << str1 << endl;
        model mod1 = build_a_model(str1);
        mod1.check_consistency();
        mod1.print();
        cout << "haha" << endl;
        sstate init = mod1.init_sstate();
        string ln="";
        for ( auto it = init.loc_names.begin(); it != init.loc_names.end(); it++)
          ln += *it;
        cout << ln << endl;
        //REQUIRE(ln=="idle");
        cout << "123"<< endl;
        cout << init.cvx << endl;
        cout << "123"<< endl;
        vector<sstate> vss = mod1.Post(init);
        for ( auto it = vss.begin(); it != vss.end(); it++) {
          ln="";
          for ( auto iit = it->loc_names.begin(); iit != it->loc_names.end(); iit++)
            ln += *iit;
          cout << ln << endl;
          cout << it->cvx << endl;
        }
        mod1.SpaceExplorer();
    }
}
