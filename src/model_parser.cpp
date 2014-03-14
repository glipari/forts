
#include "model_parser.hpp"

model model_builder::get_model()
{
  return mod;
}

void model_builder::bad_loc(automaton & a, location &l)
{
  l.bad = true;
}
void model_builder::init_loc(automaton & a, location &l)
{
  a.init_loc = make_shared<location>(l);
}

void model_builder::aton_name(tipa::parser_context &pc)
{
    auto x = pc.collect_tokens();
    if (x.size() < 1) throw string("Error in collecting variable."); 
    string an = x[x.size()-1].second;
    cout << "an : " << an << endl;
    aton_names.push_back(an);

}

void model_builder::loc_name(tipa::parser_context &pc)
{
    auto x = pc.collect_tokens();
    if (x.size() < 1) throw string("Error in collecting variable."); 
    string ln = x[x.size()-1].second;
    cout << "ln : " << ln << endl;
    loc_names.push_back(ln);
}

//void model_builder::aton_loc_pairs(void(*model_builder::fn)(automaton&, location &))
//{
//    for ( int i = 0; i < aton_names.size(); i++) {
//      string an = aton_names.at(i);
//      string ln = loc_names.at(i);
//      for ( auto it = mod.automata.begin(); it != mod.automata.end(); it++) {
//        bool aton_matched = false;
//        bool loc_matched = false;
//        if (an == it->name) {
//          aton_matched = true;
//          for ( auto jt = it->locations.begin(); jt != it->locations.end(); jt++) {
//            if ( ln == jt->name) {  
//              fn(*it, *jt);
//              //it->init_loc = make_shared<location>(*jt);
//              loc_matched = true;
//              break;
//            }
//          }
//          if ( !loc_matched)
//            throw string("No location named ") + ln + string(" in automaton ") + an;
//        }
//        if (!aton_matched)
//          throw string("No automaton named ") + an;
//      }
//    }
//    cout << "atone names size : " << aton_names.size() << endl;
//    aton_names.clear();
//    loc_names.clear();
//}

void model_builder::init_locs(tipa::parser_context &pc)
{
    for ( int i = 0; i < aton_names.size(); i++) {
      string an = aton_names.at(i);
      string ln = loc_names.at(i);
      for ( auto it = mod.automata.begin(); it != mod.automata.end(); it++) {
        bool aton_matched = false;
        bool loc_matched = false;
        if (an == it->name) {
          aton_matched = true;
          for ( auto jt = it->locations.begin(); jt != it->locations.end(); jt++) {
            if ( ln == jt->name) {  
              it->init_loc = make_shared<location>(*jt);
              loc_matched = true;
              break;
            }
          }
          if ( !loc_matched)
            throw string("No location named ") + ln + string(" in automaton ") + an;
        }
        if (!aton_matched)
          throw string("No automaton named ") + an;
      }
    }
    cout << "atone names size : " << aton_names.size() << endl;
    aton_names.clear();
    loc_names.clear();
}

void model_builder::bad_locs(tipa::parser_context &pc)
{
    for ( int i = 0; i < aton_names.size(); i++) {
      string an = aton_names.at(i);
      string ln = loc_names.at(i);
      cout << "an : " << an << endl;
      cout << "ln : " << ln << endl;
      for ( auto it = mod.automata.begin(); it != mod.automata.end(); it++) {
        bool aton_matched = false;
        bool loc_matched = false;
        if (an == it->name) {
          aton_matched = true;
          for ( auto jt = it->locations.begin(); jt != it->locations.end(); jt++) {
            if ( ln == jt->name) {  
              jt->bad = true;
              loc_matched = true;
              break;
            }
          }
          if ( !loc_matched)
            throw string("No location named ") + ln + string(" in automaton ") + an;
        }
        if (!aton_matched)
          throw string("No automaton named ") + an;
      }
    }
    aton_names.clear();
    loc_names.clear();
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
  rule r_mod, r_cvars, r_dvars, r_automaton, r_cvar, r_dvar, dv_lhs, dv_rhs, r_init, r_init_constraint, r_aton_name, r_loc_name, r_aton_loc_pair, r_bad_locs, r_init_locs;

  r_aton_name = rule(tk_ident);
  r_loc_name = rule(tk_ident);

  r_aton_loc_pair = keyword("loc") >> rule('[') >> r_aton_name >> rule(']') >> rule('=')>>rule('=') >> r_loc_name;
  r_init_locs = *(r_aton_loc_pair >> rule('&'));
  r_init_constraint = prepare_constraint_rule(m_builder.c_builder);

  r_init = keyword("init") >> rule(':')>>rule('=') 
            >> r_init_locs
            >> r_init_constraint >> rule(';');
  r_bad_locs = keyword("bad") >> rule(':') >> rule('=')
    >> -(r_aton_loc_pair >> *(rule('&')>>r_aton_loc_pair)) >> rule(';');

  r_cvar = rule(tk_ident);

  dv_lhs = rule(tk_ident);
  dv_rhs = rule(tk_int);
  r_dvar = dv_lhs >> -(rule('=')>>rule('=') >> dv_rhs);

  r_cvars = r_cvar >> *(rule(',')>>r_cvar) >> rule(':') >> keyword("continuous") >> rule(';');
  r_dvars = r_dvar >> *(rule(',')>>r_dvar) >> rule(':') >> keyword("discrete") >> rule(';');
  r_automaton = prepare_automaton_rule(m_builder.a_builder);

  r_mod = r_cvars >> -(r_dvars) >> *(r_automaton) >> r_init >> -(r_bad_locs);
  using namespace placeholders;
  r_aton_name [bind(&model_builder::aton_name, &m_builder, _1)];
  r_loc_name [bind(&model_builder::loc_name, &m_builder, _1)];
  r_init_locs [bind(&model_builder::init_locs, &m_builder, _1)];
  r_init_constraint [bind(&model_builder::the_init_constraint, &m_builder, _1)];
  r_bad_locs[bind(&model_builder::bad_locs, &m_builder, _1)];
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
