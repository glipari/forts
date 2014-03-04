#include "common.hpp"

int var_2_val (const string &var, const DVList &dvl) {

  for (auto it = dvl.begin(); it != dvl.end(); it++)
    if ( var == it->name)
      return it->valuation;
  throw string("no such a variable ") + string("\"") + var + string("\"") + string(" in the DVList.");
}

bool in_VList(const string &var, const VList &vl) {
  for (auto it = vl.begin(); it != vl.end(); it++)
    if ( var == it->name)
      return true;
  return false;
}

Variable get_variable(const string &var, const CVList &cvl) {

  int i = 0;
  for ( auto it = cvl.begin(); it != cvl.end(); it ++) {
    if ( it->name == var)
      return Variable(i);
    i++;
  }
  //cout << "variable " << var << endl;
  throw string("no such a variable ") + string("\"") + var + string("\"") + string(" in the CVList.");
}
