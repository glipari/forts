#ifndef _MODEL_PARSER_HPP_
#define _MODEL_PARSER_HPP_

#include "model.hpp"
#include "automaton_parser.hpp"

class model_builder {
  void bad_loc(automaton &a, location &l);
  void init_loc(automaton &a, location &l);
public:
  model mod;
  automaton_builder a_builder;
  constraint_builder c_builder;
  std::vector<std::string> aton_names; 
  std::vector<std::string> loc_names; 
  void aton_name(tipa::parser_context &pc);
  void loc_name(tipa::parser_context &pc);
  void aton_loc_pairs(void(*fn)(automaton &, location &));
  void bad_locs(tipa::parser_context &pc);
  void init_locs(tipa::parser_context &pc);
  void the_init_constraint(tipa::parser_context &pc);
  void a_cvar(tipa::parser_context &pc);
  void dv_lhs(tipa::parser_context &pc);
  void dv_rhs(tipa::parser_context &pc);
  void an_automaton(tipa::parser_context &pc);
  model get_model();
};

rule prepare_model_rule(model_builder &m_builder);
model build_a_model(const std::string &pc);

#endif
