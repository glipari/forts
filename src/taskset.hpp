#ifndef _TASKSET_HPP_
#define _TASKSET_HPP_

#include <vector>
#include <memory>
#include <algorithm> 
#include <iostream>
#include <random>

using namespace std;

struct Task {
  int id;
  int period;
  int deadline;
  int wcet;
  int wcrt;
  int workload;
  int prio;

  Task() {}
  //Task (int c, int d, int t) : period(t), deadline(d), wcet(c), wcrt(0), id(-1), workload(0), prio(-1) { if(wcet==1) wcet ++ ;}
  Task (int c, int d, int t) : period(t), deadline(d), wcet(c), wcrt(0), id(-1), workload(0), prio(-1) { }

};

struct TaskSet {
  std::vector<Task> tasks;

  TaskSet () {}
  TaskSet(const TaskSet &tk) {
    for ( auto it = tk.tasks.begin(); it != tk.tasks.end(); it++)
      tasks.push_back(*it);
  }

  void insert(const Task &task) { 
    tasks.push_back(task); 
    //tasks[tasks.size()-1].id = tasks.size()-1;
  }

  void set_ids() {
    for ( auto it = tasks.begin(); it != tasks.end(); it++)
      it->id = it - tasks.begin();
  }

  void replace_a_task(const Task task) {
    for ( auto it = tasks.begin(); it != tasks.end(); it++)
      if( it-> id == task.id) {
        *it = task;
        return;
      }
  }

  void remove_a_task(const Task task) {
    for ( auto it = tasks.begin(); it != tasks.end(); it++)
      if ( it->id == task.id) {
        tasks.erase(it);
        return;
      }
  }

  static bool DM(const Task &t1, const Task & t2) { return t1.deadline < t2.deadline;}
  static bool rDM(const Task &t1, const Task & t2) { return t1.deadline > t2.deadline;}
  void sort_by_rDM() { 
    sort(tasks.begin(), tasks.end(), TaskSet::rDM); 
  }
  void sort_by_DM() { 
    sort(tasks.begin(), tasks.end(), TaskSet::DM); 
    //higher priority task has lower index
    for (int i = 0; i < tasks.size(); i++)
      tasks[i].id = i;
      
  }
  void print() const {
    cout << "Total Util = " << total_utilization() << endl;
    for (auto it = tasks.begin(); it != tasks.end(); it++) {
      cout << "id : " << it->id << ", ";
      cout << "prio : " << it->prio << ", ";
      cout << "wcet : " << it->wcet << ", ";
      cout << "period : " << it->period << ", ";
      cout << "deadline : " << it->deadline << ", ";
      cout << "wcrt : " << it->wcrt << endl;
    }
  }

  double total_utilization()const {
    double sum = 0;
    for (auto it = tasks.begin(); it != tasks.end(); it++)
      sum += (float)it->wcet/it->period;
    return sum;

  }
  
};

static Task generate_a_task(int T_min, int T_max, double u_min, double u_max, double dt_min, double dt_max, unsigned seed)
{
  default_random_engine generator(seed);
  uniform_real_distribution<double> dis_T(T_min, T_max);
  uniform_real_distribution<double> dis_u(u_min, u_max);
  uniform_real_distribution<double> dis_dt(dt_min, dt_max);

  int T = dis_T(generator);
  int D = T * dis_dt(generator);
  int C = T * dis_u(generator);

  //cout << "C " << C << "D " << D << "T " << T << endl;
  return Task(C, D, T);

}

static TaskSet generate_a_taskset(int nTasks, int T_min, int T_max, double u_min, double u_max, double dt_min, double dt_max, unsigned &seed)

{
  TaskSet taskset;
  for ( int i = 0; i < nTasks; i++)
    taskset.insert( generate_a_task(T_min, T_max, u_min, u_max, dt_min, dt_max, seed++));

  return taskset;

}


int count_ones( vector<int> & v);
vector < vector<int> > generate_ci_set(int ntasks, int n_ci);

int interference_nc_new(const Task & task, int x, const Task & t_k, int h);
int interference_ci_new(const Task & task, int x, const Task & t_k, int h);
int omega_new(int x, const TaskSet & nc, const TaskSet & ci, const Task & tk, int h);
int Omega_new(const TaskSet &taskset, const Task & t_k, int M, int h, int s);
int lb_Omega_new(const TaskSet &taskset, const Task & t_k, int M, int h);

bool iteration_is_terminating(const Task &t_k, const TaskSet &taskset, int M);
TaskSet build_hp_taskset(const TaskSet & tks, const Task &t);

int RTA_NEW_WCRT(const TaskSet &hps, const Task &t, int M);
int fast_RTA_NEW_WCRT(const TaskSet &hps, const Task &t, int M);
void compute_R_bound(TaskSet &taskset, int M);
bool RTA_OC(TaskSet &taskset, int M);

//typedef pair<int, int> pair_id_diff;
//bool com(const pair_id_diff &p1, const pair_id_diff &p2)
//{
//  return p1.second > p2.second;
//}
vector<int> generate_ci_set_guan(const TaskSet & taskset, int x, const Task & tk, int M);
int RTA_GUAN_WCRT(const TaskSet &hps, const Task &t, int M);
int Omega_guan(const TaskSet &taskset, const Task & t_k, int M, int h);
int omega_new_oc(int x, const TaskSet & nc, const TaskSet & ci, const Task & tk, int h, int M);

int interference_nc_old(const Task & t_i, int x, const Task & t_k, int h);
int interference_ci_old(const Task & t_i, int x, const Task & t_k, int h);
int omega_old(int x, const TaskSet & nc, const TaskSet & ci, const Task & tk, int h);
int RTA_GUAN_WCRT_old(const TaskSet &hps, const Task &t, int M);
int Omega_guan_old(const TaskSet &taskset, const Task & t_k, int M, int h);
vector<int> generate_diff_ci_set_old(const TaskSet & taskset, int x, const Task & tk, int M, int h);
bool D_RTA_LC(TaskSet &taskset, int M, bool short_deadline);
bool D_RTA_LC_old(TaskSet &taskset, int M, bool short_deadline);

bool RTA_NEW(TaskSet &taskset, int M);
bool fast_RTA_NEW(TaskSet &taskset, int M);
bool RTA_LC(TaskSet &taskset, int M);
bool RTA_LC_old(TaskSet &taskset, int M);

static int count0 = 0;
static int count1 = 0;
int get_count0();
int get_count1();








struct State {
  /**
   * To store each task's nat and rct inside v.
   * Given a task i, nat(i) is stored in v[2*i] 
   * and rct(i) is in v[2*i+1].
   * */
  std::vector<int> v;
  bool valid;
  int signature;
  std::shared_ptr<State> parent;
  std::vector< std::shared_ptr<State> > children;

  State() : valid(true), signature(0) {}

  void reset() { valid = false; v.clear(); }

  int nat(int i) { return v[2*i]; }
  int rct(int i) { return v[2*i+1]; }

  //bool eligible(int i) { return nat(i)<=0 & rct(i)==0; }

};




#endif
