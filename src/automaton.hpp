#ifndef _AUTOMATON_HPP_
#define _AUTOMATON_HPP_

#include <tipa/tinyparser.hpp>
#include "assignment.hpp"
#include "syntax_trees.hpp"

using namespace tipa;
using namespace std;

class EDGE {
public:
  shared_ptr<constraint_node> constraint;
  vector<assignment> assignments;
  string dest;

};

EDGE build_an_edge(const string &str);

class edge_builder {
public:
  EDGE e;
  assignment_builder ass_builder;
  constraint_builder c_builder;
  EDGE get_edge() {
    return e;
  }

  void an_assignment(parser_context &pc)
  {
    e.assignments.push_back(ass_builder.get_assignment());
  }
  void the_constraint(parser_context &pc)
  {
    e.constraint = c_builder.get_tree();
  }
  void the_dest(parser_context &pc)
  {
    auto x = pc.collect_tokens();
    if (x.size() < 1) throw string("Error in collecting variable."); 
    string v = x[x.size()-1].second;
    e.dest = v;
  }

};


rule prepare_edge_rule(edge_builder &e_builder);
rule prepare_assignment_rule(edge_builder &e_builder)
{
    rule ass = prepare_assignment_rule(e_builder.ass_builder);
    using namespace placeholders;
    ass [bind(&edge_builder::an_assignment, &e_builder, _1)];
    return ass;
}

#endif
