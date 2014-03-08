#include "common.hpp"

int var_2_val (const std::string &var, const DVList &dvl) 
{
    for (auto it = dvl.begin(); it != dvl.end(); it++)
	if (var == it->name)
	    return it->valuation;
    throw std::string("no such a variable ") + std::string("\"") + var 
	+ std::string("\"") + std::string(" in the DVList.");
}

bool in_VList(const std::string &var, const VList &vl) {
    for (auto it = vl.begin(); it != vl.end(); it++)
	if (var == it->name)
	    return true;
    return false;
}

void change_var_value(const std::string &var, DVList &dv, int val)
{
    for (auto &x : dv) 
	if (x.name == var) {
	    x.valuation = val;
	    return;
	}
    throw std::string("no such a variable ") + std::string("\"") + var 
	+ std::string("\"") + std::string(" in the DVList.");
}
