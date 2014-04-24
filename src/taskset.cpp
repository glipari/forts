#include "taskset.hpp"

#include <cmath>
#include <iostream>
#include <utility>
#include <algorithm>

using namespace std;

//static int count = 0;

int get_count0()
{
  int copy = count0;
  count0 = 0;
  return copy;
}

int get_count1()
{
  int copy = count1;
  count1 = 0;
  return copy;
}

int count_ones( vector<int> & v)
{
  int res = 0;
  for (auto it = v.begin(); it != v.end(); it ++)
    if (*it>0)
      res ++;
  return res;
}

typedef pair<int, int> pair_id_diff;
static bool com(const pair_id_diff &p1, const pair_id_diff &p2)
{
  return p1.second > p2.second;
}

/**
 * Generate the CI taskset that result in least interference from higher priority tasks.
 * The interference is computed with new workload formulation.
 */
vector<int> lb_generate_diff_ci_set_new(const TaskSet & taskset, int x, const Task & tk, int M, int h)
{
  vector<int> ci;
  vector<pair_id_diff> diffs;

  for (auto it = taskset.tasks.begin(); it != taskset.tasks.end(); it++) {
    if ( interference_ci_new(*it, x, tk, h) < interference_nc_new(*it, x, tk, h))
      ci.push_back(1);
    else 
      ci.push_back(0);
  }

  return ci;

}

/**
 * Generate the CI taskset that result in largest interference from higher priority tasks.
 * The interference is computed with new workload formulation.
 */
vector<int> generate_diff_ci_set_new(const TaskSet & taskset, int x, const Task & tk, int M, int h)
{
  vector<int> ci;
  vector<pair_id_diff> diffs;

  for (auto it = taskset.tasks.begin(); it != taskset.tasks.end(); it++) {
    int diff = interference_ci_new(*it, x, tk, h)- interference_nc_new(*it, x, tk, h);
    diffs.push_back(make_pair(it-taskset.tasks.begin(), diff));
  }

  sort(diffs.begin(), diffs.end(), com);

  for (auto it = taskset.tasks.begin(); it != taskset.tasks.end(); it++) {
    ci.push_back(0);
  }

  for (int i = 0; i < M-1; i++)
    if ( diffs[i].second > 0)
      ci[diffs[i].first] = 1;
    else break;
  
  return ci;

}

/**
 * Generate all possible CI tasksets.
 */
vector < vector<int> > generate_ci_set(int ntasks, int n_ci)
{
  vector < vector<int> > v;
  vector<int> v1;
  vector<int> v2;

  if (ntasks == 1) 
  {
    v1.push_back(0);
    v2.push_back(1);
    v.push_back(v1);
    v.push_back(v2);
    return v;
  }

  vector<vector<int> > inter  = generate_ci_set(ntasks-1, n_ci);
  for ( auto it = inter.begin(); it != inter.end(); it++)
  {
    v1 = *it;
    v2 = *it;
    v1.push_back(0);
    v2.push_back(1);
    if (count_ones(v1) <= n_ci)
      v.push_back(v1);
    if (count_ones(v2) <= n_ci)
      v.push_back(v2);
  }

  return v;
}

bool iteration_is_terminating(const Task &t_k, const TaskSet &taskset, int M)
{
  double v_sum = 0;
  double u_k = (double)t_k.wcet / t_k.period;
  for ( auto it = taskset.tasks.begin(); it != taskset.tasks.end(); it ++)
    v_sum += fmin( (double)it->wcet/it->period, 1-u_k);
  return v_sum + M*u_k != M;
}

/**
 * New NC workload formulation.
 */
int interference_nc_new(const Task & t_i, int x, const Task & t_k, int h)
{
  int k = x / t_i.period;
  int Delta = x % t_i.period;
  int workload = k * t_i.wcet + fmin(Delta, t_i.wcet);

  return fmin( x - h*t_k.wcet + 1, workload); 
}

/**
 * New CI workload formulation.
 */
//int interference_ci_new(const Task & t_i, int x, const Task & t_k, int h)
//{
//  int w = ceil( (double)t_i.wcrt / t_i.period );
//  int xi = ceil( (double)(t_i.wcrt-t_i.wcet) / (t_i.period-t_i.wcet) ) - (w-1);
//  int sigma = w * t_i.wcet - 1 + (xi-1)*t_i.wcet;
//  int x_p = t_i.wcet - 1 + (w+xi-1)*t_i.period - t_i.wcrt;
//  
//  int workload;
//  if ( x < sigma)
//    workload = x;
//  else if ( x >= sigma && x < x_p)
//    workload = sigma;
//  else {
//    int k = (x-x_p) / t_i.period;
//    int Delta = (x-x_p) % t_i.period;
//    workload = sigma + k * t_i.wcet + fmin(Delta, t_i.wcet);
//  }
//
//  return fmin(x - h*t_k.wcet + 1, workload); 
//}
int interference_ci_new(const Task & t_i, int x, const Task & t_k, int h)
{
  int xp = t_i.wcet - 1 + ceil( (double)(t_i.wcrt-t_i.wcet)/(t_i.period-t_i.wcet))*t_i.period - t_i.wcrt;
  int sigma = ceil( (double)(t_i.wcrt-t_i.wcet)/(t_i.period-t_i.wcet))*t_i.wcet - 1;

  int workload;
  if ( x <= sigma)
    workload = x;
  else if ( x < xp)
    workload = sigma;
  else {
    int k = (x-xp)/t_i.period;
    workload = sigma + k*t_i.wcet + fmin((x-xp)%t_i.period, t_i.wcet);
  }

  return fmin(x - h*t_k.wcet + 1, workload); 
}

/**
 * Given a CI taskset, to compute the total interference from HP tasks.
 * New workload formulation is used here.
 */
int omega_new(int x, const TaskSet & nc, const TaskSet & ci, const Task & tk, int h)
{
  int total_interference = 0;
  for ( auto it = nc.tasks.begin(); it != nc.tasks.end(); it++)
    total_interference += interference_nc_new(*it, x, tk, h);
  for ( auto it = ci.tasks.begin(); it != ci.tasks.end(); it++)
    total_interference += interference_ci_new(*it, x, tk, h);
  return total_interference;
}

/**
 * Compute the lower bound of the fixed-point.
 */
int lb_Omega_new(const TaskSet &taskset, const Task & t_k, int M, int h)
{
  int n = taskset.tasks.size();

  int x = h * t_k.wcet;
  int X = x;
  while (true) {
    vector<int>  ci_set = lb_generate_diff_ci_set_new(taskset, x, t_k, M, h);
    TaskSet nc_taskset, ci_taskset;
    for ( int i = 0; i < n; i++)
      if ( ci_set[i] == 1)
        ci_taskset.insert(taskset.tasks[i]);
      else
        nc_taskset.insert(taskset.tasks[i]);

    x = omega_new(x, nc_taskset, ci_taskset, t_k,h) / M + h*t_k.wcet;

    if ( x - (h-1)*t_k.period > t_k.deadline)
      return x;
    if ( x == X) {
      return x;
    }

    X = x;
  }
  return X;
}

int Omega_new(const TaskSet &taskset, const Task & t_k, int M, int h, int s)
{
  int wcrt = 0;
  int n = taskset.tasks.size();

  vector< vector<int> > ci_set = generate_ci_set(n, M-1);

  for ( auto it = ci_set.begin(); it != ci_set.end(); it++) {

    TaskSet nc_taskset, ci_taskset;

    for ( int i = 0; i < n; i++)
      if ( (*it)[i] == 1)
        ci_taskset.insert(taskset.tasks[i]);
      else
        nc_taskset.insert(taskset.tasks[i]);

    int x = s; //h * t_k.wcet;
    int X = x;
    while (true) {
      x = omega_new(x, nc_taskset, ci_taskset, t_k,h) / M + h*t_k.wcet;
      //x = omega_new_oc(x, nc_taskset, ci_taskset, t_k,h,M) / M + h*t_k.wcet;
      if ( x - (h-1)*t_k.period > t_k.deadline)
        return x;
      if ( x == X) {
        wcrt = fmax(wcrt, X);
        break;
      }
      X = x;
    }

  }

  return wcrt;
}

bool fast_RTA_NEW(TaskSet &taskset, int M)
{
  for ( int i  = 0; i < taskset.tasks.size(); i ++) { 
    if ( i < M) {
      taskset.tasks[i].wcrt = taskset.tasks[i].wcet;
      continue;
    }
    TaskSet hps;
    Task &t_k = taskset.tasks[i];
    for ( int j = 0; j < i; j++)
      hps.insert(taskset.tasks[j]);
    int wcrt = fast_RTA_NEW_WCRT(hps, t_k, M);
    if ( wcrt > t_k.deadline)
      return false;
    t_k.wcrt = wcrt;
  }
  return true;
}

bool RTA_NEW(TaskSet &taskset, int M)
{
  for ( int i  = 0; i < taskset.tasks.size(); i ++) { 
    if ( i < M) {
      taskset.tasks[i].wcrt = taskset.tasks[i].wcet;
      continue;
    }
    TaskSet hps;
    Task &t_k = taskset.tasks[i];
    for ( int j = 0; j < i; j++)
      hps.insert(taskset.tasks[j]);
    int wcrt = RTA_NEW_WCRT(hps, t_k, M);
    if ( wcrt > t_k.deadline)
      return false;
    t_k.wcrt = wcrt;
  }
  return true;
}

bool RTA_LC(TaskSet &taskset, int M)
{
  for ( int i  = 0; i < taskset.tasks.size(); i ++) { 
    if ( i < M) {
      taskset.tasks[i].wcrt = taskset.tasks[i].wcet;
      continue;
    }
    TaskSet hps;
    Task &t_k = taskset.tasks[i];
    for ( int j = 0; j < i; j++)
      hps.insert(taskset.tasks[j]);
    int wcrt = RTA_GUAN_WCRT(hps, t_k, M);
    if ( wcrt > t_k.deadline)
      return false;
    t_k.wcrt = wcrt;
  }
  return true;
}

bool RTA_LC_old(TaskSet &taskset, int M)
{
  for ( int i  = 0; i < taskset.tasks.size(); i ++) { 
    if ( i < M) {
      taskset.tasks[i].wcrt = taskset.tasks[i].wcet;
      continue;
    }
    TaskSet hps;
    Task &t_k = taskset.tasks[i];
    for ( int j = 0; j < i; j++)
      hps.insert(taskset.tasks[j]);
    int wcrt = RTA_GUAN_WCRT_old(hps, t_k, M);
    if ( wcrt > t_k.deadline)
      return false;
    t_k.wcrt = wcrt;
  }
  return true;
}

bool RTA_OC(TaskSet &taskset, int M)
{
  TaskSet tks(taskset);
  
  while (tks.tasks.size() != 0) {
    bool succ = false;
    for ( int i = 0; i < tks.tasks.size(); i++) {
      Task t = tks.tasks[i];
      TaskSet hps = build_hp_taskset(tks, t);

      compute_R_bound(hps,M);
      int wcrt = RTA_NEW_WCRT(hps, t, M);
      //int wcrt = RTA_GUAN_WCRT(hps, t, M);
      if (wcrt <= t.deadline) {
        t.wcrt = wcrt;
        t.prio = hps.tasks.size();
        taskset.replace_a_task(t);
        tks.remove_a_task(t);
        succ = true;
        break;
      }

    }
    if ( ! succ)  
      return false;
  }
  return true;
}

TaskSet build_hp_taskset(const TaskSet & tks, const Task &t)
{
  TaskSet hps;
  for ( auto it = tks.tasks.begin(); it != tks.tasks.end(); it++) {
    if ( it->id != t.id)
      hps.insert(*it);
  }
  return hps;
}

//int fast_RTA_NEW_WCRT(const TaskSet &hps, const Task &t, int M)
//{
//  if ( hps.tasks.size() < M)
//    return t.wcet;
//
//  int wcrt = 0;
//  int h = 1;
//  int preX = 0;
//  while (true) {
//    int s = lb_Omega_new(hps, t, M, h);
//    if ( s < preX) {
//      //hps.insert(t);
//      cout << t.wcet << ", " << t.period << ", " << t.deadline << endl;
//      hps.print();
//      cout << "s : " << s << endl;
//      cout << "preX : " << preX << endl;
//      count0 ++;
//      //throw 0;
//    }
//    else if ( preX != 0) count1++;
//
//    if ( s - (h-1)*t.period > t.deadline)
//      return s-(h-1)*t.period;
//    int Xh = Omega_new(hps, t, M, h, s);
//    preX = Xh + t.wcet;
//    wcrt = fmax(wcrt, Xh-(h-1)*t.period);
//    if ( wcrt > t.deadline)
//      return wcrt;
//    else if (Xh-(h-1)*t.period<=t.period) {
//      return wcrt;
//    }
//    else h++;
//  }
//}

/**
 * To compute the WCRT of a task "t_k" by using RTA-NEW.
 * An efficient strating point is used for the iteration procedure.
 */
int fast_RTA_NEW_WCRT(const TaskSet &hps, const Task &t_k, int M)
{
  if ( hps.tasks.size() < M)
    return t_k.wcet;

  int wcrt = 0;
  vector<int> lb;
  // To generate all possible ci tasksets
  int n = hps.tasks.size();
  vector< vector<int> > ci_set = generate_ci_set(n, M-1);
  // Enumerate every CI taskset
  for ( auto it = ci_set.begin(); it != ci_set.end(); it++) {

    TaskSet nc_taskset, ci_taskset;

    for ( int i = 0; i < n; i++)
      if ( (*it)[i] == 1)
        ci_taskset.insert(hps.tasks[i]);
      else
        nc_taskset.insert(hps.tasks[i]);

    
    int h = 1;

    int tmp_R = 0;
    int preX = 0;
    // For each job of t_k in the problem window
    while (true) {

      if ( h > lb.size())
        lb.push_back(lb_Omega_new(hps, t_k, M, h));
      
      // The starting point is decided here
      int x = fmax(lb.at(h-1), preX+t_k.wcet); //h * t_k.wcet;
      if ( lb.at(h-1) > preX+t_k.wcet && preX > 0)
        count0 ++;
      else if (preX+t_k.wcet > lb.at(h-1))
        count1 ++;
      int X = x;
      while (true) {
        x = omega_new(x, nc_taskset, ci_taskset, t_k,h) / M + h*t_k.wcet;
        if ( x - (h-1)*t_k.period > t_k.deadline)
          return x-(h-1)*t_k.period;
        if ( x == X) {
          //wcrt = fmax(wcrt, X);
          break;
        }
        X = x;
      }
      int res = X - (h-1)*t_k.period;
      tmp_R = fmax(res, tmp_R);
      if ( res < t_k.period) {
        wcrt = fmax(wcrt, tmp_R);
        break;
      }
      else {preX = X; h++;}
    }
  }

  return wcrt;

}

int RTA_NEW_WCRT(const TaskSet &hps, const Task &t, int M)
{
  if ( hps.tasks.size() < M)
    return t.wcet;

  int wcrt = 0;
  int h = 1;
  while (true) {
    int s = h*t.wcet;
    int Xh = Omega_new(hps, t, M, h, s);
    wcrt = fmax(wcrt, Xh-(h-1)*t.period);
    if ( wcrt > t.deadline)
      return wcrt;
    else if (Xh-(h-1)*t.period<=t.period) {
      return wcrt;
    }
    else h++;
  }
}

void compute_R_bound(TaskSet &taskset, int M)
{
  for ( auto it = taskset.tasks.begin(); it != taskset.tasks.end(); it ++)
    it->wcrt = it->deadline;

  bool flag = true;
  while (flag) {
    flag = false;
    for ( auto it = taskset.tasks.begin(); it != taskset.tasks.end(); it ++) {
      TaskSet hps = build_hp_taskset(taskset, *it);
      int wcrt = it->wcrt;
      //it->wcrt = fmin(it->deadline, RTA_NEW_WCRT(hps, *it, M));
      it->wcrt = fmin(it->deadline, RTA_GUAN_WCRT(hps, *it, M));
      if ( wcrt != it->wcrt)
        flag = true;
    }
  }
}

/** 
 * This function computes the WCRT of the task "t" according to 
 * Guan's RTA with new workload formulation. 
 * */
int RTA_GUAN_WCRT(const TaskSet &hps, const Task &t, int M)
{
  if ( hps.tasks.size() < M)
    return t.wcet;

  int wcrt = 0;
  int h = 1;
  while (true) {
    // To compute the fixed-point Xh given the h
    int Xh = Omega_guan(hps, t, M, h);
    wcrt = fmax(wcrt, Xh-(h-1)*t.period);
    if ( wcrt > t.deadline)
      return wcrt;
    else if (Xh-(h-1)*t.period<=t.period) {
      return wcrt;
    }
    else h++;
  }
}

/**
 * Compute the fixed-point of Guan's iteration with new workload formulation.
 * */
int Omega_guan(const TaskSet &taskset, const Task & t_k, int M, int h)
{
  int n = taskset.tasks.size();

  int x = h * t_k.wcet;
  int X = x;
  while (true) {
    vector<int>  ci_set = generate_diff_ci_set_new(taskset, x, t_k, M, h);
    TaskSet nc_taskset, ci_taskset;
    for ( int i = 0; i < n; i++)
      if ( ci_set[i] == 1)
        ci_taskset.insert(taskset.tasks[i]);
      else
        nc_taskset.insert(taskset.tasks[i]);

    x = omega_new(x, nc_taskset, ci_taskset, t_k,h) / M + h*t_k.wcet;

    if ( x - (h-1)*t_k.period > t_k.deadline)
      return x;
    if ( x == X) {
      return x;
    }

    X = x;
  }

}

//typedef pair<int, int> pair_ii;
//static bool com_less(const pair_ii &p1, const pair_ii &p2)
//{
//  return p1.second < p2.second;
//}
//int omega_new_oc(int x, const TaskSet & nc, const TaskSet & ci, const Task & tk, int h, int M)
//{
//  int total_interference = 0;
//  TaskSet tks;
//  vector< pair_ii > v;
//  vector< pair_ii > v_p;
//
//  for ( auto it = nc.tasks.begin(); it != nc.tasks.end(); it++) {
//    tks.insert(*it);
//    v.push_back(make_pair(it->id, interference_nc_new(*it, x, tk, h)));
//  }
//  for ( auto it = ci.tasks.begin(); it != ci.tasks.end(); it++) {
//    tks.insert(*it);
//    v.push_back(make_pair(it->id, interference_ci_new(*it, x, tk, h)));
//  }
//
//  sort(v.begin(), v.end(), com);
//
//  for ( auto it = tks.tasks.begin(); it != tks.tasks.end(); it++) {
//    it->wcrt = it->wcet;
//    v_p.push_back(make_pair(it->id, fmax(interference_nc_new(*it, x, tk, h), interference_nc_new(*it, x, tk, h))));
//  }
//  
//  sort(v_p.begin(), v_p.end(), com_less);
//
//  for ( int i = 0; i < v.size() - M; i++)
//    total_interference += v[i].second;
//  for ( int i = v_p.size() - M; i < v_p.size(); i++)
//    total_interference += v_p[i].second;
//
//  return total_interference;
//}

//int omega_new_oc(int x, const TaskSet & nc, const TaskSet & ci, const Task & tk, int h, int M)
//{
//  int total_interference = 0;
//  TaskSet tks;
//  vector< pair_ii > v;
//  vector< pair_ii > v_p;
//
//  for ( auto it = nc.tasks.begin(); it != nc.tasks.end(); it++) {
//    int inter = interference_nc_new(*it, x, tk, h);
//    v.push_back(make_pair(it->id, inter));
//    Task t = *it;
//    t.wcrt = t.wcet;
//    v_p.push_back(make_pair(it->id, inter - interference_nc_new(t, x, tk, h)));
//  }
//  for ( auto it = ci.tasks.begin(); it != ci.tasks.end(); it++) {
//    int inter = interference_ci_new(*it, x, tk, h);
//    v.push_back(make_pair(it->id, inter));
//    Task t = *it;
//    t.wcrt = t.wcet;
//    v_p.push_back(make_pair(it->id, inter - interference_ci_new(t, x, tk, h)));
//  }
//
//  sort(v_p.begin(), v_p.end(), com_less);
//
//  for ( int i = 0; i < M; i++) {
//    for ( auto it = v.begin(); it != v.end(); it++)
//      if( it->first == v_p[i].first)
//        it->second = it->second - v_p[i].second;
//  }
//
//  for ( auto it = v.begin(); it != v.end(); it++)
//    total_interference += it->second;
//  return total_interference;
//}

/************************ The original RTA_LC ***************************/

int interference_nc_old(const Task & t_i, int x, const Task & t_k, int h)
{
  int k = x / t_i.period;
  int Delta = x % t_i.period;
  int workload = k * t_i.wcet + fmin(Delta, t_i.wcet);

  return fmin( x - h*t_k.wcet + 1, workload); 
}

int interference_ci_old(const Task & t_i, int x, const Task & t_k, int h)
{
  int above = fmax(x-t_i.wcet, 0);
  int tmp1 = above % t_i.period - (t_i.period - t_i.wcrt);
  int alpha = fmin( fmax(tmp1,0), t_i.wcet-1 );
  int workload = (above/t_i.period)*t_i.wcet + t_i.wcet + alpha;;

  return fmin(x - h*t_k.wcet + 1, fmax(workload, 0)); 
}

int omega_old(int x, const TaskSet & nc, const TaskSet & ci, const Task & tk, int h)
{
  int total_interference = 0;
  for ( auto it = nc.tasks.begin(); it != nc.tasks.end(); it++)
    total_interference += interference_nc_old(*it, x, tk, h);
  for ( auto it = ci.tasks.begin(); it != ci.tasks.end(); it++)
    total_interference += interference_ci_old(*it, x, tk, h);
  return total_interference;
}

int RTA_GUAN_WCRT_old(const TaskSet &hps, const Task &t, int M)
{
  if ( hps.tasks.size() < M)
    return t.wcet;

  int wcrt = 0;
  int h = 1;
  while (true) {
    int Xh = Omega_guan_old(hps, t, M, h);
    wcrt = fmax(wcrt, Xh-(h-1)*t.period);
    if ( wcrt > t.deadline)
      return wcrt;
    else if (Xh-(h-1)*t.period<=t.period) {
      return wcrt;
    }
    else h++;
  }
}

int Omega_guan_old(const TaskSet &taskset, const Task & t_k, int M, int h)
{
  int n = taskset.tasks.size();

  int x = h * t_k.wcet;
  int X = x;
  while (true) {
    vector<int>  ci_set = generate_diff_ci_set_old(taskset, x, t_k, M, h);
    TaskSet nc_taskset, ci_taskset;
    for ( int i = 0; i < n; i++)
      if ( ci_set[i] == 1)
        ci_taskset.insert(taskset.tasks[i]);
      else
        nc_taskset.insert(taskset.tasks[i]);

    x = omega_old(x, nc_taskset, ci_taskset, t_k,h) / M + h*t_k.wcet;

    if ( x - (h-1)*t_k.period > t_k.deadline)
      return x;
    if ( x == X) {
      return x;
      //wcrt = fmax(wcrt, X);
      //break;
    }

    X = x;
  }

}

vector<int> generate_diff_ci_set_old(const TaskSet & taskset, int x, const Task & tk, int M, int h)
{
  vector<int> ci;
  vector<pair_id_diff> diffs;

  for (auto it = taskset.tasks.begin(); it != taskset.tasks.end(); it++) {
    int diff = interference_ci_old(*it, x, tk, h)- interference_nc_old(*it, x, tk, h);
    diffs.push_back(make_pair(it-taskset.tasks.begin(), diff));
  }

  sort(diffs.begin(), diffs.end(), com);

  for (auto it = taskset.tasks.begin(); it != taskset.tasks.end(); it++) {
    ci.push_back(0);
  }

  for (int i = 0; i < M-1; i++)
    if ( diffs[i].second > 0)
      ci[diffs[i].first] = 1;
    else break;
  
  return ci;

}

bool D_RTA_LC(TaskSet &taskset, int M, bool short_deadline)
{
  TaskSet tks(taskset);
  
  while (tks.tasks.size() != 0) {
    bool succ = false;
    for ( int i = 0; i < tks.tasks.size(); i++) {
      Task t = tks.tasks[i];
      TaskSet hps = build_hp_taskset(tks, t);

      for (auto it = hps.tasks.begin(); it != hps.tasks.end(); it ++)
        it->wcrt = it->deadline;

      int wcrt = RTA_GUAN_WCRT(hps, t, M);
      //int wcrt;
      //if (short_deadline)
      //  wcrt = RTA_GUAN_WCRT_old(hps, t, M);
      //else 
      //  wcrt = RTA_GUAN_WCRT(hps, t, M);
      if (wcrt <= t.deadline) {
        t.wcrt = wcrt;
        t.prio = hps.tasks.size();
        taskset.replace_a_task(t);
        tks.remove_a_task(t);
        succ = true;
        break;
      }

    }
    if ( ! succ)  
      return false;
  }
  return true;
}

bool D_RTA_LC_old(TaskSet &taskset, int M, bool short_deadline)
{
  TaskSet tks(taskset);
  
  while (tks.tasks.size() != 0) {
    bool succ = false;
    for ( int i = 0; i < tks.tasks.size(); i++) {
      Task t = tks.tasks[i];
      TaskSet hps = build_hp_taskset(tks, t);

      for (auto it = hps.tasks.begin(); it != hps.tasks.end(); it ++)
        it->wcrt = it->deadline;

      //int wcrt = RTA_NEW_WCRT(hps, t, M);
      int wcrt;
      //if (short_deadline)
        wcrt = RTA_GUAN_WCRT_old(hps, t, M);
      //else 
        //wcrt = RTA_GUAN_WCRT(hps, t, M);
      if (wcrt <= t.deadline) {
        t.wcrt = wcrt;
        t.prio = hps.tasks.size();
        taskset.replace_a_task(t);
        tks.remove_a_task(t);
        succ = true;
        break;
      }

    }
    if ( ! succ)  
      return false;
  }
  return true;
}
/************************ END of the original RTA_LC ***************************/
