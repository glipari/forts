#include "automaton.hpp"

using namespace tipa;
using namespace std;


edge edge_builder::get_edge() 
{
    return e;
}

void edge_builder::an_assignment(parser_context &pc)
{
    e.assignments.push_back(ass_builder.get_assignment());
}

void edge_builder::the_guard(parser_context &pc)
{
    e.guard = c_builder.get_tree();
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
    r_edge = r_when >> r_constraint >> r_do >> rule('{') >> r_ass >> *(rule(',') >> r_ass) >> rule('}') >> r_goto >> r_dest >> rule(';');
    r_ass = prepare_assignment_rule(e_builder);
    r_constraint = prepare_constraint_rule(e_builder.c_builder);

    using namespace placeholders;
    r_constraint [bind(&edge_builder::the_guard, &e_builder, _1)];
    r_dest [bind(&edge_builder::the_dest, &e_builder, _1)];
    return r_edge;
}

edge build_an_edge(const string &expr_input)
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

location location_builder::get_location() 
{
    return loc;
}

void location_builder::a_rate(parser_context &pc)
{
    loc.rates.push_back(rate_builder.get_assignment());
}

void location_builder::an_outgoing(parser_context &pc)
{
    loc.outgoings.push_back(outgoing_builder.get_edge());
    outgoing_builder = edge_builder();
}

void location_builder::the_invariant(parser_context &pc)
{
    loc.invariant = invariant_builder.get_tree();
}

void location_builder::the_name(parser_context &pc)
{
    auto x = pc.collect_tokens();
    if (x.size() < 1) throw string("Error in collecting variable."); 
    string v = x[x.size()-1].second;
    loc.name = v;
}

rule prepare_rate_rule(location_builder &loc_builder)
{
    rule rate = prepare_assignment_rule(loc_builder.rate_builder);
    using namespace placeholders;
    rate [bind(&location_builder::a_rate, &loc_builder, _1)];
    return rate;
}

rule prepare_outgoing_rule(location_builder &loc_builder)
{
    rule outgoing = prepare_edge_rule(loc_builder.outgoing_builder);
    using namespace placeholders;
    outgoing [bind(&location_builder::an_outgoing, &loc_builder, _1)];
    return outgoing;
}

rule prepare_location_rule(location_builder & loc_builder)
{
    rule r_loc, r_while, r_constraint, r_ass, r_wait, r_l, r_name, r_outgoing;

    r_outgoing = prepare_outgoing_rule(loc_builder);
    r_l = rule("loc", true);
    r_while = rule("while", true);
    r_wait= rule("wait", true);
    r_name = rule(tk_ident);
    r_loc = r_l >> r_name >> rule(':') >> r_while >> r_constraint >> r_wait >> rule('{') >> r_ass >> *(rule(',') >> r_ass) >> rule('}') >> *r_outgoing;
    r_ass = prepare_rate_rule(loc_builder);
    r_constraint = prepare_constraint_rule(loc_builder.invariant_builder);

    using namespace placeholders;
    r_constraint [bind(&location_builder::the_invariant, &loc_builder, _1)];
    r_name [bind(&location_builder::the_name, &loc_builder, _1)];
    return r_loc;
}

location build_a_location(const string &expr_input)
{
    location_builder loc_builder;
    rule r_loc = prepare_location_rule(loc_builder);

    using namespace std::placeholders;

    stringstream str(expr_input);

    parser_context pc;
    pc.set_stream(str);

    bool f = false;
    try {
	f = r_loc.parse(pc);
    } catch (parse_exc &e) {
	cout << "Parse exception!" << endl;
    }

    if (!f) throw pc.get_formatted_err_msg();
    else return loc_builder.get_location();
}

