#include <iostream>
#include <fstream>
#include <model.hpp>
#include "expression.hpp"
#include "sstate.hpp"
#include <model_parser.hpp>
#include <thread> 

using namespace std;

int main(int argc, char *argv[])
{
    if (argc == 1) {
	cout << "Usage: " << argv[0] << " [options] <filename> " << endl;
        return 0;
    }
    
    MODEL.reset();

    /** argv[1] in the input file name. */
    ifstream ifs(argv[1]);
    
    if (not ifs.good()) {
	cout << "Could not read " << argv[1] << endl;
	return 0;
    }

    string str((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>()); 

    build_a_model(str);
    try {
        MODEL.check_consistency();
        /** To print out the model? */
        //MODEL.print();
        MODEL.set_sstate_type(BOX_WIDENED);
        //MODEL.set_sstate_type(WIDENED);
        //MODEL.SpaceExplorer();
	MODEL.set_concurrency(2);
	MODEL.SpaceExplorerParallel();
        MODEL.print_log();
    } catch(const string &s) {
        cout << s << endl;
        return 1;
    }


    return 0;
}
