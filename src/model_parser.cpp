#include "model_parser.hpp"

using namespace std;

void model_builder::aton_name(tipa::parser_context &pc)
{
    auto x = pc.collect_tokens();
    if (x.size() < 1) throw parse_exc("Error in collecting variable."); 
    string an = x[x.size()-1].second;
    cout << "an : " << an << endl;
    aton_names.push_back(an);
}

void model_builder::loc_name(tipa::parser_context &pc)
{
    auto x = pc.collect_tokens();
    if (x.size() < 1) throw parse_exc("Error in collecting variable."); 
    string ln = x[x.size()-1].second;
    cout << "ln : " << ln << endl;
    loc_names.push_back(ln);
}

void model_builder::init_locs(tipa::parser_context &pc)
{
    for (unsigned i = 0; i < aton_names.size(); i++) {
        // throws if not found
	automaton &autom = MODEL.get_automaton_by_name(aton_names.at(i));
	// throws if not found
	autom.set_init_location(loc_names.at(i)); 
	cout << "Setting init location for automaton " 
	     << autom.get_name() << " == " 
	     << loc_names.at(i) << " / " 
	     << autom.get_init_location() << endl;	    
    }
    cout << "aton names size : " << aton_names.size() << endl;
    aton_names.clear();
    loc_names.clear();
}

void model_builder::bad_locs(tipa::parser_context &pc)
{
    for (unsigned i = 0; i < aton_names.size(); i++) {
	string an = aton_names.at(i);
	string ln = loc_names.at(i);
	cout << "an : " << an << endl;
	cout << "ln : " << ln << endl;
	automaton &autom = MODEL.get_automaton_by_name(an);
	Location &loc = autom.get_location_by_name(ln);
	loc.set_bad(true);
    }
    aton_names.clear();
    loc_names.clear();
}

void model_builder::the_init_constraint(tipa::parser_context &pc)
{
    MODEL.set_init(c_builder.get_tree());
    c_builder = constraint_builder();
}

void model_builder::a_cvar(tipa::parser_context &pc)
{
    auto x = pc.collect_tokens();
    if (x.size() < 1) throw parse_exc("Error in collecting variable."); 
    string v = x[x.size()-1].second;
    MODEL.add_cvar(v);
}

void model_builder::dv_lhs(tipa::parser_context &pc)
{
    auto x = pc.collect_tokens();
    if (x.size() < 1) throw parse_exc("Error in collecting variable."); 
    string v = x[x.size()-1].second;
    //mod.dvars.insert(make_pair(v, 0));//push_back(variable(v));
    last_dvar_name = v;
}

void model_builder::dv_rhs(tipa::parser_context &pc)
{
    auto x = pc.collect_tokens();
    if (x.size() < 1) throw parse_exc("Error in collecting variable."); 
    string v = x[x.size()-1].second;
    MODEL.add_dvar(last_dvar_name, atoi(v.c_str()));
}

void model_builder::an_automaton(tipa::parser_context &pc)
{
    automaton aton = a_builder.get_automaton();
    MODEL.add_automaton(aton);
    cout << "automaton name : " << aton.get_name() << endl;
    a_builder = automaton_builder();
}

rule prepare_model_rule(model_builder &m_builder)
{
    rule r_mod, r_cvars, r_dvars, r_automaton, 
	r_cvar, r_dvar, dv_lhs, dv_rhs, r_init, 
	r_init_constraint, 
	r_aton_name, r_loc_name, r_aton_loc_pair, 
	r_bad_locs, r_init_locs;

    r_aton_name = rule(tk_ident);
    r_loc_name =  rule(tk_ident);

    r_aton_loc_pair   = keyword("loc") >> rule('[') >> r_aton_name >> rule(']') 
				       >> rule("==") >> r_loc_name;

    r_init_locs       = *(r_aton_loc_pair >> rule('&'));
    r_init_constraint = prepare_constraint_rule(m_builder.c_builder);

    r_init            = keyword("init") >> rule(":=") 
					>> r_init_locs
					>> r_init_constraint >> rule(';');

    r_bad_locs        = keyword("bad") >> rule(":=") 
				       >> -(r_aton_loc_pair >> *(rule('&') >> r_aton_loc_pair)) 
				       >> rule(';');

    r_cvar = rule(tk_ident);

    dv_lhs = rule(tk_ident);
    dv_rhs = rule(tk_int);
    //r_dvar = dv_lhs >> -(rule("==") >> dv_rhs);
    r_dvar = rule(tk_ident) >> -(rule("==") >> rule(tk_int));

    r_cvars     = r_cvar >> *(rule(',') >> r_cvar) >> rule(':') >> keyword("continuous") >> rule(';');
    r_dvars     = r_dvar >> *(rule(',') >> r_dvar) >> rule(':') >> keyword("discrete")   >> rule(';');

    r_automaton = prepare_automaton_rule(m_builder.a_builder);

    r_mod       = r_cvars >> -(r_dvars) >> *(r_automaton) >> r_init >> -(r_bad_locs);

    using namespace placeholders;
    r_aton_name       [bind(&model_builder::aton_name,           &m_builder, _1)];
    r_loc_name        [bind(&model_builder::loc_name,            &m_builder, _1)];
    r_init_locs       [bind(&model_builder::init_locs,           &m_builder, _1)];
    r_init_constraint [bind(&model_builder::the_init_constraint, &m_builder, _1)];
    r_bad_locs        [bind(&model_builder::bad_locs,            &m_builder, _1)];
    r_cvar            [bind(&model_builder::a_cvar,              &m_builder, _1)];
    dv_lhs            [bind(&model_builder::dv_lhs,              &m_builder, _1)];
    dv_rhs            [bind(&model_builder::dv_rhs,              &m_builder, _1)];
    r_automaton       [bind(&model_builder::an_automaton,        &m_builder, _1)];

    return r_mod;
}

void build_a_model(const std::string &input)
{
    model_builder m_builder;
    rule r_mod = prepare_model_rule(m_builder);

    using namespace std::placeholders;

    stringstream str(input);

    parser_context pc;
    pc.set_stream(str);

    cout << "- Ready to parse the model" << endl;

    bool f = false;
    try {
	f = r_mod.parse(pc);
    } catch (parse_exc &e) {
	cout << "Parse exception!" << endl;
    } catch (...) {
	cout << "Unexpected exception!" << endl;
    }

    if (!f) throw parse_exc(pc.get_formatted_err_msg());
}
