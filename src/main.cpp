#include <iostream>
#include <fstream>
#include <model.hpp>
#include "expression.hpp"
#include "sstate.hpp"
#include <model_parser.hpp>

using namespace std;

int main(int argc, char *argv[])
{
    if (argc == 1)
        return 0;
    
    MODEL.reset();
    /** argv[1] in the input file name. */
    ifstream ifs(argv[1]);
    string str((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>()); 

    build_a_model(str);
    try {
        MODEL.check_consistency();

        /** To print out the model? */
        MODEL.print();
        
        MODEL.SpaceExplorer();        

        MODEL.print_log();
    } catch(const string &s) {
        cout << s << endl;
        return 1;
    }


    return 0;
}
