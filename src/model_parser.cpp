
#include "model_parser.hpp"

model model_builder::get_model()
{
  return mod;
}

void model_builder::aton_name(tipa::parser_context &pc)
{
    auto x = pc.collect_tokens();
    if (x.size() < 1) throw string("Error in collecting variable."); 
    string an = x[x.size()-1].second;
    aton_names.push_back(an);

}

void model_builder::aton_loc_pair(tipa::parser_context &pc)
{
    auto x = pc.collect_tokens();
    if (x.size() < 1) throw string("Error in collecting variable."); 
    string ln = x[x.size()-1].second;
    string an = aton_names.back();
    cout << "an : " << an << endl;
    cout << "ln : " << ln << endl;
    //loc_names.push_back(ln);
    for ( auto it = mod.automata.begin(); it != mod.automata.end(); it++) {
      if (an == it->name) {
        it->init_loc_name = ln;
        cout << it->name << "---" << ln << "---" << it->init_loc_name << endl;
      }
    }
}

void model_builder::the_init_constraint(tipa::parser_context &pc)
{
  mod.init_constraint = c_builder.get_tree();
  c_builder = constraint_builder();
}

void model_builder::a_cvar(tipa::parser_context &pc)
{
    auto x = pc.collect_tokens();
    if (x.size() < 1) throw string("Error in collecting variable."); 
    string v = x[x.size()-1].second;
    mod.cvars.push_back(variable(v));
}

void model_builder::dv_lhs(tipa::parser_context &pc)
{
    auto x = pc.collect_tokens();
    if (x.size() < 1) throw string("Error in collecting variable."); 
    string v = x[x.size()-1].second;
    mod.dvars.push_back(variable(v));
}

void model_builder::dv_rhs(tipa::parser_context &pc)
{
    auto x = pc.collect_tokens();
    if (x.size() < 1) throw string("Error in collecting variable."); 
    string v = x[x.size()-1].second;
    mod.dvars.back().set_val(atoi(v.c_str()));
}

void model_builder::an_automaton(tipa::parser_context &pc)
{
  automaton aton = a_builder.get_automaton();
  mod.automata.push_back(aton);
  cout << "automaton name : " << aton.name << endl;
  a_builder = automaton_builder();
}

rule prepare_model_rule(model_builder &m_builder)
{
  rule r_mod, r_cvars, r_dvars, r_automaton, r_cvar, r_dvar, dv_lhs, dv_rhs, r_init, r_init_constraint, r_aton_name, r_loc_name, r_aton_loc_pair;

  r_aton_name = rule(tk_ident);
  r_loc_name = rule(tk_ident);

  r_aton_loc_pair = keyword("loc") >> rule('[') >> r_aton_name >> rule(']') >> rule('=')>>rule('=') >> r_loc_name;
  r_init_constraint = prepare_constraint_rule(m_builder.c_builder);

  r_init = keyword("init") >> rule(':')>>rule('=') 
            >> *(r_aton_loc_pair >> rule('&')) 
            >> r_init_constraint >> rule(';');

  r_cvar = rule(tk_ident);

  dv_lhs = rule(tk_ident);
  dv_rhs = rule(tk_int);
  r_dvar = dv_lhs >> -(rule('=')>>rule('=') >> dv_rhs);

  r_cvars = r_cvar >> *(rule(',')>>r_cvar) >> rule(':') >> keyword("continuous") >> rule(';');
  r_dvars = r_dvar >> *(rule(',')>>r_dvar) >> rule(':') >> keyword("discrete") >> rule(';');
  r_automaton = prepare_automaton_rule(m_builder.a_builder);

  r_mod = r_cvars >> -(r_dvars) >> *(r_automaton) >> r_init;
  using namespace placeholders;
  r_aton_name [bind(&model_builder::aton_name, &m_builder, _1)];
  r_loc_name [bind(&model_builder::aton_loc_pair, &m_builder, _1)];
  r_init_constraint [bind(&model_builder::the_init_constraint, &m_builder, _1)];
  r_cvar [bind(&model_builder::a_cvar, &m_builder, _1)];
  dv_lhs [bind(&model_builder::dv_lhs, &m_builder, _1)];
  dv_rhs [bind(&model_builder::dv_rhs, &m_builder, _1)];
  r_automaton [bind(&model_builder::an_automaton, &m_builder, _1)];

  return r_mod;
}

model build_a_model(const std::string &input)
{
    model_builder m_builder;
    rule r_mod = prepare_model_rule(m_builder);

    using namespace std::placeholders;

    stringstream str(input);

    parser_context pc;
    pc.set_stream(str);

    bool f = false;
    try {
	f = r_mod.parse(pc);
    } catch (parse_exc &e) {
	cout << "Parse exception!" << endl;
    }

    if (!f) throw pc.get_formatted_err_msg();
    else return m_builder.get_model();
}
