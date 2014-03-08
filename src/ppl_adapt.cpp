#include "ppl_adapt.hpp"

using namespace std;

PPL::Variable get_variable(const string &var, const CVList &cvl) {
    int i = 0;
    for ( auto it = cvl.begin(); it != cvl.end(); it ++) {
	if (it->name == var)
	    return PPL::Variable(i);
	i++;
    }
    throw string("no such a variable ") + string("\"") + var + string("\"") + string(" in the CVList.");
}
