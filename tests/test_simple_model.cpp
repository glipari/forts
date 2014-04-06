#include "catch.hpp" 

#include <fstream>
#include <iostream>
#include <string>
#include <model.hpp>
#include <model_parser.hpp>
#include <constraint_parser.hpp>

using namespace Parma_Polyhedra_Library::IO_Operators;
using namespace std;
using namespace tipa;

Location &parse_location(const std::string &auto_loc)
{
    std::string a, l;

    tipa::rule name = tipa::rule(tk_ident) >> 
	tipa::rule('.') >> tipa::rule(tk_ident);
    
    auto f = [&a, &l](parser_context &pc) {
	auto v = pc.collect_tokens();
	if (v.size() != 2) throw tipa::parse_exc("malformed location name");
	a = v[0].second;
	l = v[1].second;
    };
    name[ f ];
    stringstream str {auto_loc};
    parser_context pc;
    pc.set_stream(str);
    name.parse(pc);
    
    automaton &at = MODEL.get_automaton_by_name(a);
    Location  &lc = at.get_location_by_name(l);
    return lc;
}


Symbolic_State build_state(const std::vector<std::string> &locs, 
			   const Valuations  &dv,
			   const std::string &constraints)
{
    // std::vector<Location *> locs;
    // for (auto x : auto_locs) {
    // 	auto y = parse_location(x);
    // 	locs.push_back(&y);
    // }
    auto cv = MODEL.get_cvars();
    auto cs = build_a_constraint_tree(constraints);
    PPL::C_Polyhedron cvx(cv.size());
    cvx.add_constraints(cs.to_Linear_Constraint(cv, dv));
    return Symbolic_State { locs, dv, cvx };
    
}

bool compare_state_sets(const list<Symbolic_State> &la,
		       const list<Symbolic_State> &lb)
{
    int c = 0;
    for (auto &x : la) 
	if (find(begin(lb), end(lb), x) != end(lb))
	    c++;
	else {
	    cout << "State not found: " << endl;
	    x.print();
	}
	    
    if (c == la.size()) return true;
    else return false;
}


TEST_CASE("Simple model", "[model][Space]")
{
    Model::reset();
    ifstream ifs("sm.forts");
    string str {std::istreambuf_iterator<char>(ifs), 
	    std::istreambuf_iterator<char>()};

    cout << "------------------ File has been read ----------------------" << endl;
    cout << str << endl;
    cout << "------------------------------------------------------------" << endl;
    build_a_model(str);
    cout << "------------------ Model has been built --------------------" << endl;
    MODEL.check_consistency(); // TODO put this inside build_a_model();
    cout << "------------------ Consistency checked ---------------------" << endl;
    MODEL.print();

    MODEL.SpaceExplorer();

    // build expected states

    Symbolic_State s_a = build_state({ "LOC_A" }, 
				     {{"d", 0}}, 
				     "x == y & x <= 2 & y >=0"); 
    Symbolic_State s_b = build_state({ "LOC_B" }, 
				     {{"d", 0}}, 
				     "x == 0 & y >=2");
    Symbolic_State s_c = build_state({ "LOC_C" }, 
				     {{"d", 0}}, 
				     "x == 0 & y <=5 & y >= 2");

    list<Symbolic_State> expected = { s_a, s_b, s_c };

    auto li = MODEL.get_all_states();
    for (auto x : li) x.print();

    CHECK(compare_state_sets(li, expected));
}
