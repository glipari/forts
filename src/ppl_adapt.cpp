#include "ppl_adapt.hpp"

using namespace std;

PPL::Variable get_variable(const string &var, const CVList &cvl) {
    int i = 0;
    for ( auto it = cvl.begin(); it != cvl.end(); it ++) {
	if (it->name == var)
	    return PPL::Variable(i);
	i++;
    }
    auto s = string("no such a variable \"") + var + string("\" in the CVList.");
    throw s;
}
