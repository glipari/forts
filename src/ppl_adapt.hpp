#ifndef __PPL_ADAPT_HPP__
#define __PPL_ADAPT_HPP__

#include <common.hpp>

#include <ppl.hh>
namespace PPL = Parma_Polyhedra_Library;

// A few typedefs...
typedef PPL::Constraint_System   Linear_Constraint;
typedef PPL::Constraint          AT_Constraint;
typedef PPL::Linear_Expression   Linear_Expr;
typedef PPL::Interval<int64_t, PPL::Interval_Info_Null<PPL::Rational_Interval_Info_Policy> > rt_r_oc;
typedef PPL::Box<rt_r_oc> TBox;

// returns the PPL variable associated to the string in CVList
// I do not like this function, it assumes the CVList is properly ordered... 
PPL::Variable get_ppl_variable(const VariableList &cvl, const std::string &var);


#endif
