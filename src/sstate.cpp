#include <sstate.hpp>
#include <iostream>
#include <ppl.hh>
using namespace std;
using namespace Parma_Polyhedra_Library::IO_Operators;

bool sstate::contains(const sstate &ss) const
{
  for ( int i = 0; i < loc_names.size(); i++)
    if ( loc_names[i] != ss.loc_names[i])
      return false;
  return cvx.contains(ss.cvx);
}

void sstate::print() 
{
  cout << "-----------------------------" << endl;
  cout << "State name : ";
  for ( auto &n : loc_names)
    cout << n;
  cout << endl;
  cout << "CVX : " << endl;
  cout << cvx << endl;
  cout << endl;
  cout << "-----------------------------" << endl;
}
