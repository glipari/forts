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

void run_tests(int CPUs, int nTasks, int beginning_util, int beginning_taskset_index, int beginning_rta_sched, int beginning_forts_sched);

int main(int argc, char** argv)
{

    int CPUs, nTasks, beginning_util=0, beginning_taskset_index=0, beginning_rta_sched=0, beginning_forts_sched=0;
    int c = -1;
    while ((c = getopt(argc, argv, "m:n:u:t:r:f:")) != -1) {
        if (c == 'm') {
            CPUs = atoi(optarg);
        }
        if (c == 'n') {
            nTasks = atoi(optarg);
        }
        if (c == 'u') {
            beginning_util = atoi(optarg);
        }
        if (c == 't') {
            beginning_taskset_index = atoi(optarg);
        }
        if (c == 'r') {
            beginning_rta_sched = atoi(optarg);
        }
        if (c == 'f') {
            beginning_forts_sched = atoi(optarg);
        }
    }
    run_tests(CPUs, nTasks, beginning_util, beginning_taskset_index, beginning_rta_sched, beginning_forts_sched);
}

void run_tests(int CPUs, int nTasks, int beginning_util, int beginning_taskset_index, int beginning_rta_sched, int beginning_forts_sched)
{

    int nSets;
    vector<int> utils; 
    for ( int i = 0; i <= CPUs*10/2; i++)
        utils.push_back(CPUs*10/2+i);

    for ( auto util : utils) {
        if ( util < beginning_util )
            continue;

        nSets = 100;
        int rta_schedulable = 0, forts_schedulable = 0 ;
        // the file to print schedulability result
        string SchedResultsName = string("sched-res-m") + to_string(CPUs) + string("-n") + to_string(nTasks) + string("-u")+to_string(util)+string(".dat");
        // another output file to print details for FORTS schedulability check results
        string FortsResultsName = string("forts-res-m") + to_string(CPUs) + string("-n") + to_string(nTasks) + string("-u")+to_string(util)+string(".dat");
        // the file to read task sets
        string TasksReaderName = string("./tasksets-c/m") + to_string(CPUs) + string("-n") + to_string(nTasks) + string("-u")+to_string(util) + string(".dat");      

        ifstream TasksReader(TasksReaderName);
        for ( int i = 0; i < nSets; i++) {
            // to read a task set, which has nTasks tasks inside
            TaskSet tks;
            string line;
            for ( int j = 0; j < nTasks; j++) {
                int wcet, period;
                getline(TasksReader, line);
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
            getline(TasksReader, line);
            if ( util == beginning_util && i < beginning_taskset_index) {
                rta_schedulable = beginning_rta_sched;
                forts_schedulable = beginning_forts_sched;
                continue;
            }

            // for each task we read, we now assign deadline to it
            for ( int k = 0; k < tks.tasks.size(); k++) {
                Task &t = tks.tasks[k];
                int period = t.period;
                seed = (t.period + t.wcet)*123*(k+10);
                default_random_engine generator(seed);
                uniform_real_distribution<double> dis_o(0.8,1);
                t.deadline = period*dis_o(generator);
                if (t.deadline < t.wcet)
                    t.deadline = t.wcet;
            }
            // sort the tasks by DM
            tks.sort_by_DM();
            // first apply RTA-LC on the task set
            bool rta_succ = RTA_LC(tks, CPUs);
            // if current task set is schedulable under RTA-LC, there is no need to call FORTS
            if (rta_succ) {
                ofstream of;
                of.open(SchedResultsName, ios::app);
                of << rta_schedulable++ << "  " << forts_schedulable++ << "  " << i << endl;
                of.close();
                continue;
            }

            // RTA-LC fails and weare going to try FORTS
            string template_model_name = string("../models/") + to_string(nTasks) + string("t-") + to_string(CPUs) + string("p.forts");
            std::ifstream ifs(template_model_name);
            std::string str((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>()); 
            cout << "Starting parsing the input ... ";
            MODEL.reset();
            build_a_model(str);

            bool succ = false;
            try {
                // replace task parameters in MODEL with what we just read
                for ( int k = 0; k < tks.tasks.size(); k++) {
                    Task &t = tks.tasks[k];
                    set_valuation(MODEL.get_dvars(), "T"+to_string(k+1), t.period);
                    set_valuation(MODEL.get_dvars(), "D"+to_string(k+1), t.deadline);
                    set_valuation(MODEL.get_dvars(), "C"+to_string(k+1), t.wcet);
                }

                MODEL.check_consistency();
                MODEL.set_sstate_type(BOX_WIDENED);
                MODEL.SpaceExplorer();
                if (MODEL.unknown != "unknown") {
                    forts_schedulable ++;
                    succ = true;
                }
            } catch(const string &s) {
                cout << s << endl;
            } catch(const char* s) { cout << s << endl;}

            int64_t mem = MODEL.total_memory_in_bytes()/(1024*1024);

            ofstream of;
            // to print FORTS checking details
            of.open(FortsResultsName, ios::app);
            of << "time " << MODEL.se_stat.get_total() << " memory  " << mem << " steps " << MODEL.steps << "   states    " << MODEL.stats.final_states << "    schedulable " << succ << "  " << MODEL.unknown << endl;
            of.close();
            // to print the schedulability results
            of.open(SchedResultsName, ios::app);
            of << rta_schedulable << "  " << forts_schedulable << "  " << i << endl;
            of.close();

            if ( MODEL.unknown == "unknown")
                nSets ++;
            else 
                i -- ;
        }

        // if there is no schedulable task sets in current utilization level, we do not further explore higher utilization levels
        if (forts_schedulable == 0)
            return;
    }

}
