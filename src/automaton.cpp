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

