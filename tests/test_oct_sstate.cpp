#include "catch.hpp" 

#include <fstream>
#include <iostream>
#include <string>
#include <model.hpp>
#include <model_parser.hpp>
#include <constraint_parser.hpp>
#include <oct_sstate.hpp>

using namespace Parma_Polyhedra_Library::IO_Operators;
using namespace std;
using namespace tipa;

static Location &parse_location(const std::string &auto_loc)
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


static shared_ptr<OCT_Symbolic_State> build_state(const std::vector<std::string> &locs, 
			   const Valuations  &dv,
			   const std::string &constraints)
{
    auto cv = MODEL.get_cvars();
    auto cs = build_a_constraint_tree(constraints);
    PPL::NNC_Polyhedron cvx(cv.size());
    cvx.add_constraints(cs.to_Linear_Constraint(cv, dv));
    auto res = make_shared<OCT_Symbolic_State> (locs, dv, cvx );
    res->continuous_step();
    return res;
    
}

static bool compare_state_sets(const list<Symbolic_State> &la,
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

static bool compare_state_sets(const list<shared_ptr<Symbolic_State> > &la,
		       const list<Symbolic_State>  &lb)
{
    int c = 0;
    for (auto &x : la) 
	if (find(begin(lb), end(lb), *x) != end(lb)) {
	    c++;
    }
	else {
	    cout << "State not found: " << endl;
	    x->print();
	}
	 
    if (c == la.size()) return true;
    else return false;
}



TEST_CASE("Simple OCT model", "[OCT]")
{
    Model::reset();
    ifstream ifs("oct-test.forts");
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

    MODEL.set_sstate_type(OCT);
    MODEL.SpaceExplorer();

    // build expected states

    auto s_a = build_state({ "A", "D" }, 
				     {}, 
				     "x<=2 & x >= 0 & x==y"); 
    auto s_b = build_state({ "B", "D" }, 
				     {}, 
				     "x>=0 & y-x==2");
    auto s_c = build_state({ "C", "D1" }, 
				     {}, 
				     "x>=0 & y-x==0");

    //list<Symbolic_State> expected = { *s_a, *s_b, *s_c};



    auto li = MODEL.get_all_states();
    auto it = li.begin();
    auto s1 = *it;
    it ++;
    auto s2 = *it;
    it ++;
    auto s3 = *it;
    REQUIRE (s_a->equals(s1));
    REQUIRE (s_b->equals(s2));
    REQUIRE (s_c->equals(s3));
    //cout << "The states we got : " << endl;
    
    //for (auto x : li) 
    //    x->print();

}