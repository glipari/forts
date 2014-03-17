#ifndef _MODEL_HPP_
#define _MODEL_HPP_

#include "automaton.hpp"

typedef constraint_node constraint;

class model {
public:
  CVList cvars;
  DVList dvars;
  std::shared_ptr<constraint>  init_constraint; 
  std::vector<automaton> automata;
  void print();
  void check_consistency();
};




int myfunction(int x);

#endif
