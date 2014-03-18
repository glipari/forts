#ifndef _MODEL_HPP_
#define _MODEL_HPP_

#include "automaton.hpp"
#include "sstate.hpp"
#include  <vector>
#include  <list>

typedef constraint_node constraint;

class model {
public:
  CVList cvars;
  DVList dvars;
  std::shared_ptr<constraint>  init_constraint; 
  std::vector<automaton> automata;
  void print();
  void check_consistency();
  void continuous_step(sstate &ss);
  void discrete_step(sstate &ss, const std::vector<edge> &edges);
  std::vector<sstate> Post(const sstate& ss);
  PPL::C_Polyhedron get_invariant_cvx(sstate &ss);
  sstate init_sstate();
  bool is_bad(const sstate &ss);
  /** The symbolic state space */
  std::list<sstate> Space;
  /** To explore Space according to breadth first search. */
  void SpaceExplorer();
};




int myfunction(int x);

#endif
