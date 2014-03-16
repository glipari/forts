#include "automaton.hpp"

using namespace std;

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


void location::print() 
{
    std::cout << "loc " << name << ": while ";
    invariant->print();
    std::cout << " wait {";
    for ( auto it = rates.begin(); it != rates.end(); it++) {
	if ( it != rates.begin())
	    cout << ", ";
	it->print();
    }
    std::cout << " }\n";
    for (auto it = outgoings.begin(); it != outgoings.end(); it++)
	it->print();
}

void automaton::print()
{
  std::cout << "automaton " << name << std::endl;
  std::cout << "sync: ";
  for ( auto it = labels.begin(); it != labels.end(); it++) {
    if ( it != labels.begin())
      std::cout << ", ";
    std::cout << *it ;
  }
  std:: cout << ";" << std::endl;
  for ( auto it = locations.begin(); it != locations.end(); it++) {
    it->print();
    std::cout << std::endl;
  }
  std:: cout << "end" << std::endl;
}

bool automaton::check_consistency(const CVList &cvl, const DVList &dvl)
{
  for (auto it = locations.begin(); it != locations.end(); it++) {
    /** Check consistency of the invariant in each location. */
    auto tt = it->invariant;
    tt->to_Linear_Constraint(cvl, dvl);

    for ( auto iit = it->outgoings.begin(); iit != it->outgoings.end(); iit++){

      /** Check consistency of the guard in each edge. */
      auto tt = iit->guard;
      tt->to_Linear_Constraint(cvl, dvl);

      /** The destination location must exist. */
      string dest = iit->dest;
      bool dest_matched = false;
      for ( auto tt = locations.begin(); tt != locations.end(); tt++)
        if ( dest == tt->name) {
          dest_matched = true;
          break;
        }
      if ( !dest_matched)
        throw string("The destination \"") + dest + string("\" does not exist.");

      /** The sync lab must exist. */
      string sync_label = iit->sync_label;
      if ( sync_label == "")
        continue;
      bool lab_matched = false;
      for ( auto tt = labels.begin(); tt != labels.end(); tt++)
        if ( sync_label == *tt) {
          lab_matched = true;
          break;
        }
      if ( !lab_matched)
        throw string("The sync label \"") + sync_label + string("\" does not exist.");

    }
  }
  return true;
}
