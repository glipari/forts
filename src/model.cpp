#include <expression.hpp>
#include <model.hpp>

#include <iostream>
using namespace std;

void model::print()
{
  for (auto it = cvars.begin(); it != cvars.end(); it++)
  {
    if (it != cvars.begin())
      cout << ",";
    cout << it->name;
  }
  cout << ": continous;" << endl;
  for (auto it = dvars.begin(); it != dvars.end(); it++)
  {
    if (it != dvars.begin())
      cout << ",";
    cout << it->name;
  }
  cout << ": discrete;" << endl;
  for (auto it = automata.begin(); it != automata.end(); it++)
    it->print();
  cout << endl;
  cout << "init := " << endl;
  for ( auto it = automata.begin(); it != automata.end(); it++)
  {
    cout << "loc[" << it->name << "]==" << it->init_loc_name <<"& ";
  }
  if ( init_constraint != nullptr)
    init_constraint->print();
  cout << ";" << endl;
}

int myfunction(int a)
{
    return -1;
}
