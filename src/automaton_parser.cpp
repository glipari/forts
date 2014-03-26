#include <automaton_parser.hpp>

using namespace std;

edge edge_builder::get_edge() 
{
    return edge(dest, label, guard, ass);
}

void edge_builder::an_assignment(parser_context &pc)
{
    ass.push_back(ass_builder.get_assignment());
}

void edge_builder::the_guard(parser_context &pc)
{
    guard = c_builder.get_tree();
}

void edge_builder::the_dest(parser_context &pc)
{
    auto x = pc.collect_tokens();
    if (x.size() < 1) throw parse_exc("Error in collecting variable."); 
    dest = x[x.size()-1].second;
//    e.dest = v;
}

void edge_builder::the_sync_label(parser_context &pc)
{
    auto x = pc.collect_tokens();
    if (x.size() < 1) throw string("Error in collecting variable."); 
    label = x[x.size()-1].second;
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
    rule r_edge, r_when, r_constraint, r_ass, r_do, r_goto, r_dest, r_sync, r_label;

    r_when  = keyword("when");
    r_do    = keyword("do");
    r_goto  = keyword("goto");
    r_dest  = rule(tk_ident);
    r_label = rule(tk_ident);
    r_sync  = keyword("sync") >> r_label;
    r_edge  = r_when >> r_constraint >> -r_sync >> r_do >> rule('{') 
		     >> -(r_ass >> *(rule(',') >> r_ass)) >> rule('}') 
		     >> r_goto >> r_dest >> rule(';');
    r_ass   = prepare_assignment_rule(e_builder);
    r_constraint = prepare_constraint_rule(e_builder.c_builder);

    using namespace placeholders;
    r_constraint [bind(&edge_builder::the_guard,      &e_builder, _1)];
    r_dest       [bind(&edge_builder::the_dest,       &e_builder, _1)];
    r_label      [bind(&edge_builder::the_sync_label, &e_builder, _1)];
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

    if (!f) throw parse_exc(pc.get_formatted_err_msg());
    else return e_builder.get_edge();
}

location location_builder::get_location() 
{
    //string ln = loc.name;
    //string input =string("when true do {} goto "); 
    //input += ln;
    //input += ";";
    //edge e = build_an_edge(input);
    //loc.outgoings.push_back(e);
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
    if (x.size() < 1) throw parse_exc("Error in collecting variable."); 
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
    rule r_loc, r_while, r_constraint, r_ass, r_wait, r_l, r_name, r_outgoing, r_emptyset;

    r_outgoing = prepare_outgoing_rule(loc_builder);
    r_l        = keyword("loc");
    r_while    = keyword("while");
    r_wait     = keyword("wait");
    r_emptyset = rule('{') >> rule('}');
    r_name     = rule(tk_ident);
    r_loc      = r_l >> r_name >> rule(':') 
		     >> r_while >> r_constraint 
		     >> r_wait >> (r_emptyset | (rule('{') >> r_ass >> *(rule(',') >> r_ass) >> rule('}'))) 
		     >> *r_outgoing;

    r_ass      = prepare_rate_rule(loc_builder);
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

    if (!f) throw parse_exc(pc.get_formatted_err_msg());
    else return loc_builder.get_location();
}

automaton automaton_builder::get_automaton()
{
    return aton;
}

void automaton_builder::the_name(parser_context &pc)
{
    auto x = pc.collect_tokens();
    if (x.size() < 1) throw parse_exc("Error in collecting variable."); 
    string v = x[x.size()-1].second;
    aton.name = v;
}

void automaton_builder::a_label(parser_context &pc)
{
    auto x = pc.collect_tokens();
    if (x.size() < 1) throw parse_exc("Error in collecting variable."); 
    string v = x[x.size()-1].second;
    aton.labels.push_back(v);
}

void automaton_builder::a_location(parser_context &pc)
{
  aton.locations.push_back(l_builder.get_location());
  l_builder = location_builder();
}

rule prepare_automaton_rule(automaton_builder & a_builder)
{
    rule r_auto, r_head, r_lab, r_name, r_sync, r_loc, r_end;

    r_head = keyword("automaton");
    r_name = rule(tk_ident);
    r_lab  = rule(tk_ident);
    r_sync = keyword("sync") >> rule(':') 
			     >> -( r_lab >> *(rule(',')>>r_lab))
			     >> rule(';');
    r_loc  = prepare_location_rule(a_builder.l_builder);
    r_end  = keyword("end");

    r_auto = keyword("automaton") >> r_name >> -r_sync >> *(r_loc) 
				  >> keyword("end");

    using namespace placeholders;
    r_name [bind(&automaton_builder::the_name,   &a_builder, _1)];
    r_lab  [bind(&automaton_builder::a_label,    &a_builder, _1)];
    r_loc  [bind(&automaton_builder::a_location, &a_builder, _1)];
    return r_auto;
}

automaton build_an_automaton(const string &expr_input)
{
    automaton_builder a_builder;
    rule r_auto = prepare_automaton_rule(a_builder);

    using namespace std::placeholders;

    stringstream str(expr_input);

    parser_context pc;
    pc.set_stream(str);

    bool f = false;
    try {
	f = r_auto.parse(pc);
    } catch (parse_exc &e) {
	cout << "Parse exception!" << endl;
    }

    if (!f) throw parse_exc(pc.get_formatted_err_msg());
    else return a_builder.get_automaton();
}
