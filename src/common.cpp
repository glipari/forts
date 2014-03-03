#include "common.hpp"

int var_2_val (string var, const DVList_ptr vt) {
  if ( vt == nullptr)
    throw string("null DVList_ptr.");

  for (auto it = vt->begin(); it != vt->end(); it++)
    if ( var == it->name)
      return it->valuation;
  throw string("Unexpected variable.");
}

bool in_DVList(string var, const DVList_ptr vt) {
  if (vt == nullptr)
    return false;
  for (auto it = vt->begin(); it != vt->end(); it++)
    if ( var == it->name)
      return true;
  return false;
}

Variable get_variable(const string var, const CVList_ptr cv) {
  if ( cv == nullptr)
    throw string("null CVList_ptr.");

  int i = 0;
  for ( auto it = cv->begin(); it != cv->end(); it ++) {
    if ( it->name == var)
      return Variable(i);
    i++;
  }
  //cout << "variable " << var << endl;
  throw string("no such a variable ") + string("\"") + var + string("\"") + string(" in the CVList.");
}

