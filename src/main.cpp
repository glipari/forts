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
  /**
   * ce : the counter example guided approach
   * visual : to visulise the result by outputing graph
   **/
  bool ce = false, beep = false, visual = false, on_the_fly = false;
  /** "bound" specifies the maximum step for the analysis */
  int bound=-1;
  /** the "graph" name for plotting */
  string visual_fname;

  if (argc == 1) {
    cout << "Usage: " << argv[0] << " [options] <filename> " << endl;
    return 0;
  }
    
  MODEL.reset();
    
  int c = -1;
  int parall = 1;
  string state_type = "";
  string log_fname = "";
  while ((c = getopt(argc, argv, "p:s:r:v:f:b:l:")) != -1) {	
    /**
     * p: to run the software in parallelism, which never works ... 
     **/
    //if (c == 'p') {
    //  cout << "Parallelism set to :";
    //  parall = atoi(optarg);
    //  cout << parall << endl;
    //}
    /** there is a series of state types */
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
    if (c == 'l') {
      log_fname = string(optarg);
    }
    /** to set up the bound on steps */
    //if ( c == 'b') {
    //    bound = atoi(optarg);
    //}
    /** the robustness analysis for the automaton model with parameters */
    //if (c == 'r') {
    //  string robustness_type = string(optarg);
    //  if (robustness_type == "beep") {
    //    beep = true;
    //  }
    //  else if (robustness_type == "ce") {
    //    ce = true;
    //  }
    //  else {
    //    cout << "Unknown type for robustness analysis : " << robustness_type << endl;
    //    cout << "Possible values are : beep" << endl;
    //    return 0;
    //  }
    //}
    /** to graphically visulise the result */
    //if (c == 'v') {
    //  visual = true;
    //  visual_fname = string(optarg);
    //}
    ///** is the flag "on-the-fly" meaningful now? */	
    //if (c == 'f') {
    //    on_the_fly = true;
    //        //visual_fname = string(optarg);
    //}
  }

  /** argv[1] in the input file name. */
  ifstream ifs(argv[optind]);
  if (not ifs.good()) {
    cout << "Could not read " << argv[1] << endl;
    return 0;
  }

  string str((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>()); 
  build_a_model(str);
    
  //if ( bound != -1) {
  //  MODEL.set_bound(bound);
  //}

  try {
    //if ( beep) {
    //  MODEL.efficient_BEEP();
    //  if( visual) 
    //    MODEL.efficient_print_points(visual_fname);
    //}
    //else if (ce) {
    //  MODEL.CE();
    //  if( visual) 
    //    MODEL.print_points_ce(visual_fname);
    //}
    //else {
      MODEL.check_consistency();
      MODEL.SpaceExplorer();
      if ( not (log_fname == ""))
        MODEL.print_log(log_fname);
    //}
    //MODEL.print_log();
  } catch(const string &s) {
    cout << s << endl;
    return 1;
  } catch(const char* s) {
    cout << s << endl;
    return 1;
  }

  return 0;
}
