#include "signature.hpp"
#include <vector>

using namespace std;


const std::string& Signature::get_str() const
{
    return str;
}

const unsigned& Signature::get_active_tasks() const
{
    return active_tasks;
}

Signature::Signature(const string &s) {
    str = s;
    active_tasks = 0;

    // To parse the list of active tasks
    vector<string> acts;
    //vector<int> acts_i;
    string act = "";
    for ( auto it = s.begin(); it != s.end(); it ++) {
        if ( *it <= '9' && *it >= '0') {
            //cout << "Got one active task " << *it << endl;
            act.push_back(*it);
        }
        else if ( act != "") {
            acts.push_back(act);
            act = "";
        }
    }
    if ( act != "")
        acts.push_back(act);

    for (auto &x : acts)
        active_tasks = active_tasks | 1 << atoi(x.c_str());
}

bool Signature::is_active(const int id) const 
{
  //for ( int i = 1; i <= 10; i++) {
  unsigned ats = active_tasks;
  ats = ats & (1 << id);
  if ( ats != 0)
    return true;
  else
    return false;
}

unsigned Signature::get_lowest_count() const 
{
  for ( int i = 1; i <= 10; i++) {
    unsigned ats = active_tasks;
    ats = ats & (1 << i);
    if ( ats != 0)
      return i;
  }
  return 0;
}

unsigned Signature::get_highest_count() const 
{
  for ( int i = 10; i >= 1; i--) {
    unsigned ats = active_tasks;
    ats = ats & (1 << i);
    if ( ats != 0)
      return i;
  }
  return 0;
}

bool Signature::includes(const Signature& sig) const
{
    return active_tasks == (active_tasks | sig.active_tasks);
}

bool Signature::operator == (const Signature& sig) const
{
    return str == sig.str;
}

bool Signature::operator < (const Signature& sig) const
{
    return str < sig.str;
}

