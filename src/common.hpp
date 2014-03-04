#ifndef _COMMON_HPP_
#define _COMMON_HPP_

#include <ppl.hh>
#include <utility>
#include <list>
using namespace std;

//using namespace Parma_Polyhedra_Library;
namespace PPL = Parma_Polyhedra_Library;
//namespace PPL_IO = Parma_Polyhedra_Library::IO_Operators;

// A VARIABLE is a pair (variable name, integer valuation)
//typedef pair<string, int> VARIABLE; 
struct variable {
  variable() {}
  variable(string n) : name(n) {}
  variable(string n, int v) : name(n), valuation(v) {}
  string name;
  int valuation;
};
// The variable table for discrete variables
typedef list<variable> DVList;
// The continuous variable list
typedef list<variable> CVList;
// 
typedef list<variable> VList;

int var_2_val(const string &var, const DVList &dvl);

Variable get_variable(const string &var, const CVList &cvl);

bool in_VList(const string &var, const VList &dvl);

typedef PPL::Constraint_System Linear_Constraint;
typedef PPL::Constraint AT_Constraint;
typedef PPL::Linear_Expression Linear_Expr;
typedef PPL::Variable Variable;


#endif
