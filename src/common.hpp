#ifndef _COMMON_HPP_
#define _COMMON_HPP_

#include <list>
#include <string>

// A VARIABLE is a pair (variable name, integer valuation)
// typedef pair<string, int> VARIABLE; 
struct variable {
    variable(const std::string &n) : name(n) {}
    variable(const std::string &n, int v) : name(n), valuation(v) {}

    std::string name;
    int valuation;
};

// The variable table for discrete variables
typedef std::list<variable> DVList;
// The continuous variable list
typedef std::list<variable> CVList;
// TODO ???? what is this? 
typedef std::list<variable> VList;

// returns the value associated to a discrete variable
int var_2_val(const std::string &var, const DVList &dvl);

// changes a value in the list
void change_var_value(const std::string &var, DVList &dv, int val);

// true is the variable is present in the list
bool in_VList(const std::string &var, const VList &dvl);

#endif
