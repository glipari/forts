#ifndef __ASSIGNMENT_HPP__
#define __ASSIGNMENT_HPP__

#include <tipa/tinyparser.hpp>
#include <common.hpp>
#include <expression.hpp>

using namespace std;
using namespace tipa;

/**
   This class represents an assignment.
 */
struct assignment {
    std::string x;
    std::shared_ptr<expr_tree_node> expr;
};


class assignment_builder { 
public:
  assignment a;
  expr_builder b;
  void var_name(parser_context &pc) {
    auto v = pc.collect_tokens();
    a.x = v[v.size()-1].second;
    //a.expr = b.get_tree();
  }

  assignment get_assignment() {
    a.expr = b.get_tree();
    return a;
  }
  
};

assignment build_assignment(const std::string &str);
rule prepare_assignment_rule(assignment_builder &ab);

#endif
