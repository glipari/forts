#include <iostream>

#include "edge.hpp"
#include "unique_index.hpp"

using namespace std;

Linear_Constraint edge::guard_to_Linear_Constraint(const CVList &cvl, const DVList &dvl) const
{
  return guard->to_Linear_Constraint(cvl, dvl);
}

Linear_Constraint edge::ass_to_Linear_Constraint(const CVList &cvl, const DVList &dvl) const
{
  Linear_Constraint lc;

  for ( auto it = cvl.begin(); it != cvl.end(); it++) {
    string x = it->name;
    for ( auto iit = assignments.begin(); iit != assignments.end(); iit++) {
      if ( x == iit->x) {
        PPL::Variable v = get_variable(x, cvl);
        Linear_Expr le = iit->expr->to_Linear_Expr(cvl, dvl);
        AT_Constraint atc = (v==le);
        lc.insert(atc);
        break;
      }
    }

  }

  return lc;
}

void edge::print() 
{
    std::cout << "when ";
    guard->print();
    cout << " sync " << sync_label;
    std::cout << " do {";
    for ( auto it = assignments.begin(); it != assignments.end(); it++) {
	if ( it != assignments.begin())
	    cout << ", ";
	it->print();
    }
    std::cout << " }";
    std::cout << " goto " << dest << ";\n";
}

edge::edge() : a_index(0)
{
  index = UniqueIndex::get_next_index();
}

void edge::set_automata_index(int a)
{
    a_index = a;
}

