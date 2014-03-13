#include "automaton.hpp"

using namespace std;

void edge::print() 
{
    std::cout << "when ";
    guard->print();
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

