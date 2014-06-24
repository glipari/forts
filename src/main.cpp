#include "constraint_parser.hpp"
#include "assignment_parser.hpp"
#include "automaton_parser.hpp"
#include <string>
#include <fstream>
#include "model_parser.hpp"
#include "model.hpp"
#include <time.h>
#include "taskset.hpp"
#include "common.hpp"
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/foreach.hpp>
using namespace Parma_Polyhedra_Library::IO_Operators;
using namespace std;

unsigned seed = 12345;

int main(int argc, char** argv)
{
  int nSets;// = 100;
  int nTasks = 5; //atoi(argv[1]);
  int m = 2;
  //vector<int> utils = {15, 16, 17, 18, 19, 20};
  //vector<int> utils = {10, 11, 12, 13, 14, 16, 17, 18, 19, 20};
  vector<int> utils = {14, 15, 16, 17, 18, 19, 20};

  for ( auto util : utils) {
    nSets = 100;
    int rta_schedulable = 0, forts_schedulable = 0, sim=0, total_count=1;
    ifstream input_tks(string("../models/rtns2014/m2-n5-100-1000-dat/m2-n")+to_string(nTasks)+string("-u")+to_string(util)+string(".dat"));
    int ini = 0;
    for ( int i = ini; i < nSets; i++) {
      TaskSet tks;
      string line;
      for ( int j = 0; j < nTasks; j++) {
        int wcet, period;
        getline(input_tks, line);
        stringstream linestream(line);
        string data;
        getline(linestream, data, '\n');

        vector<string> tokens;
        boost::split(tokens, data, boost::is_any_of(" "));
        wcet = atoi(tokens[1].c_str());
        period = atoi(tokens[2].c_str());

        Task t(wcet, period, period);
        tks.insert(t);
      }
      // the delimiter line
      getline(input_tks, line);
        if ( util == 14 and i == 0) { 
            nSets ++;
            continue;
        }


      for ( int k = 0; k < tks.tasks.size(); k++) {
        Task &t = tks.tasks[k];
        int period = t.period;
        seed = (t.period + t.wcet)*123*(k+10);
        default_random_engine generator(seed);
        uniform_real_distribution<double> dis_o(0.8,1);
        t.deadline = period*dis_o(generator);
        if (t.deadline <= t.wcet)
            //t.deadline = t.wcet;
            t.deadline = t.period;
      }
      tks.sort_by_DM();

      bool rta_succ = RTA_NEW(tks, m);
      bool forts_succ = true;
      if (rta_succ) {
          rta_schedulable ++;
      }

      if (rta_succ) {
          forts_schedulable ++;
      }
      else {
          cout << "sim's turn" << endl;
          tks.print();
        clock_t begin, end;
        
        std::string temp = string("../models/rtns2014/m") + to_string(2) + string("-n")+to_string(nTasks)+string(".forts");
        std::ifstream ifs(temp);
        std::string str((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>()); 
        cout << "Starting parsing the input ... ";
        MODEL.reset();
        build_a_model(str);
        try {
          // replace task parameters in MODEL with tks
          for ( int k = 0; k < nTasks; k++) {
              Task &t = tks.tasks[k];
              int period = t.period;
              int deadline = t.deadline;
              int wcet = t.wcet;
              set_valuation(MODEL.get_dvars(), "T"+to_string(k+1), period);
              set_valuation(MODEL.get_dvars(), "D"+to_string(k+1), deadline);
              set_valuation(MODEL.get_dvars(), "C"+to_string(k+1), wcet);
          }

          MODEL.check_consistency();
          /** To print out the model? */
          MODEL.set_sstate_type(BOX_WIDENED);
          //MODEL.set_sstate_type(DBM);
          MODEL.SpaceExplorer();
          //MODEL.print_log();
        } catch(const string &s) {
            if( s == "unknown") {
                total_count ++;
                nSets ++;
                continue;
            }
          forts_succ = false;
        } catch(const char* s) {
              forts_succ = false;
        }
        
        if( forts_succ) {
            forts_schedulable ++;
        }
      }


      ofstream of2;
      of2.open(string("100-1000-ns-res-sched-m2-n")+to_string(nTasks)+string("-u")+to_string(util)+string(".dat"), ios::app);
      of2 << rta_schedulable << "  " << forts_schedulable << "  " << total_count++ << endl;
      if( rta_succ != forts_succ) {
          cout << "util : " << util << " i : " <<  i << endl;
        tks.print();
      }
      of2.close();
    }
    if (forts_schedulable == 0)
      return 0;
  }

}
