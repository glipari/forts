#include <iostream>
#include <fstream>
#include <model.hpp>
#include "expression.hpp"
#include "sstate.hpp"
#include <model_parser.hpp>
#include <thread> 

#include <unistd.h>

using namespace std;

int main(int argc, char *argv[])
{
    if (argc == 1) {
	cout << "Usage: " << argv[0] << " [options] <filename> " << endl;
        return 0;
    }
    
    MODEL.reset();
    int c = -1;
    int parall = 1;
    string state_type = "";
    while ((c = getopt(argc, argv, "p:s:")) != -1) {
	if (c == 'p') {
	    cout << "Parallelism set to :";
	    parall = atoi(optarg);
	    cout << parall << endl;
	}
	if (c == 's') {
	    state_type = string(optarg);
	    if (state_type == "box") 
		    MODEL.set_sstate_type(BOX_WIDENED);
	    else if (state_type == "widened") 
		    MODEL.set_sstate_type(WIDENED);
	    else if (state_type == "dbm") 
		    MODEL.set_sstate_type(DBM);
	    else if (state_type == "oct") 
		    MODEL.set_sstate_type(OCT);
	    else if (state_type == "origin")
		    MODEL.set_sstate_type(ORIGIN);
	    else {
		    cout << "Unknown state type : " << state_type << endl;
		    cout << "Possible values are: origin, widened, box_widened, dbm" << endl;
            return 0;
	    }
	    cout << "State set to :" << state_type << endl;
	}
    }

    /** argv[1] in the input file name. */
    ifstream ifs(argv[optind]);
    
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
//        MODEL.set_sstate_type(BOX_WIDENED);
//        MODEL.set_sstate_type(WIDENED);
        MODEL.SpaceExplorer();
	// MODEL.set_concurrency(2);
        MODEL.print_log();
    } catch(const string &s) {
        cout << s << endl;
        return 1;
    } catch(const char* s) {
        cout << s << endl;
        return 1;
    }

    return 0;
}
