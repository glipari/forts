#include "ppl_adapt.hpp"

using namespace std;

PPL::Variable get_ppl_variable(const VariableList &cvl, const string &var) {
    int i = 0;
    for (auto const &x : cvl) {
	if (x == var)
	    return PPL::Variable(i);
	i++;
    }
    auto s = string("no such a variable \"") + var + string("\" in the CVList.");
    throw s;
}
