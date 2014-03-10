#include "automaton.hpp"

using namespace tipa;
using namespace std;


EDGE edge_builder::get_edge() 
{
    return e;
}

void edge_builder::an_assignment(parser_context &pc)
{
    e.assignments.push_back(ass_builder.get_assignment());
}

void edge_builder::the_constraint(parser_context &pc)
{
    e.constraint = c_builder.get_tree();
}

void edge_builder::the_dest(parser_context &pc)
{
    auto x = pc.collect_tokens();
    if (x.size() < 1) throw string("Error in collecting variable."); 
    string v = x[x.size()-1].second;
    e.dest = v;
}

rule prepare_assignment_rule(edge_builder &e_builder)
{
    rule ass = prepare_assignment_rule(e_builder.ass_builder);
    using namespace placeholders;
    ass [bind(&edge_builder::an_assignment, &e_builder, _1)];
    return ass;
}

rule prepare_edge_rule(edge_builder & e_builder)
{
    rule r_edge, r_when, r_constraint, r_ass, r_do, r_goto, r_dest;

    r_when = rule("when", true);
    r_do= rule("do", true);
    r_goto= rule("goto", true);
    r_dest = rule(tk_ident);
    r_edge = r_when >> r_constraint >> r_do >> rule('{') >> r_ass >> *(rule(',') >> r_ass) >> rule('}') >> r_goto >> r_dest;
    r_ass = prepare_assignment_rule(e_builder);
    r_constraint = prepare_constraint_rule(e_builder.c_builder);

    using namespace placeholders;
    r_constraint [bind(&edge_builder::the_constraint, &e_builder, _1)];
    r_dest [bind(&edge_builder::the_dest, &e_builder, _1)];
    return r_edge;
}

EDGE build_an_edge(const string &expr_input)
{
    edge_builder e_builder;
    rule r_edge = prepare_edge_rule(e_builder);

    using namespace std::placeholders;

    stringstream str(expr_input);

    parser_context pc;
    pc.set_stream(str);

    bool f = false;
    try {
	f = r_edge.parse(pc);
    } catch (parse_exc &e) {
	cout << "Parse exception!" << endl;
    }

    if (!f) throw pc.get_formatted_err_msg();
    else return e_builder.get_edge();
}
