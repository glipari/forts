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


shared_ptr<Symbolic_State> build_state(const std::vector<std::string> &locs, 
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
    PPL::NNC_Polyhedron cvx(cv.size());
    cvx.add_constraints(cs.to_Linear_Constraint(cv, dv));
    auto res = make_shared<Symbolic_State> (locs, dv, cvx );
    res->continuous_step();
    return res;
    
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

bool compare_state_sets(const list<shared_ptr<Symbolic_State> > &la,
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

    auto s_a = build_state({ "LOC_A" }, 
				     {{"d", 0}}, 
				     "x == y & x <= 2 & y >=0"); 
    auto s_b = build_state({ "LOC_B" }, 
				     {{"d", 0}}, 
				     "x == 0 & y >=2");
    auto s_c = build_state({ "LOC_C" }, 
				     {{"d", 0}}, 
				     "x == 0 & y <=5 & y >= 2");

    list<Symbolic_State> expected = { *s_a, *s_b, *s_c };

    auto li = MODEL.get_all_states();
    for (auto x : li) x->print();

    CHECK(compare_state_sets(li, expected));

    Signature sig_a("LOC_A"), sig_b("LOC_B"), sig_c("LOC_C");
    REQUIRE(s_a->get_signature() == sig_a);
    REQUIRE(s_b->get_signature() == sig_b);
    REQUIRE(s_c->get_signature() == sig_c);
}

TEST_CASE("Simple model2", "[model][Space]")
{
    Model::reset();
    ifstream ifs("sm2.forts");
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

    auto s_a = build_state({ "LOC_A" }, 
				     {}, 
				     "x+y==0 & x == z & x >=0"); 
    auto s_b = build_state({ "LOC_B" }, 
				     {}, 
				     "x+z<=10 & x-y==4 & x>= 2 & x==z");

    list<Symbolic_State> expected = { *s_a, *s_b };

    auto li = MODEL.get_all_states();
    for (auto x : li) x->print();

    CHECK(compare_state_sets(li, expected));

    Signature sig_a("LOC_A"), sig_b("LOC_B");
    REQUIRE(s_a->get_signature() == sig_a);
    REQUIRE(s_b->get_signature() == sig_b);
}

TEST_CASE("Simple model3", "[model][Space]")
{
    Model::reset();
    ifstream ifs("sm3.forts");
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

    auto s_a = build_state({ "LOC_A" }, 
				     {{"T",10}, {"O",0}}, 
				     "x == 0"); 
    auto s_b = build_state({ "LOC_B" }, 
				     {{"T",10}, {"O",0}}, 
				     "x <= 10 & x >= 0");

    list<Symbolic_State> expected = { *s_a, *s_b };

    auto li = MODEL.get_all_states();
    for (auto x : li) x->print();

    CHECK(compare_state_sets(li, expected));

}

TEST_CASE("Simple water monitor model", "[model][Space]")
{
    Model::reset();
    ifstream ifs("water-level.forts");
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

    auto s_a = build_state({ "l0" }, 
				     {}, 
				     "w<=10 & w >= 1 & x >= 0 & w-x<=1"); 
    auto s_b = build_state({ "l1" }, 
				     {}, 
				     "x<=2 & x>=0 & w==x+10");
    auto s_c = build_state({ "l2" }, 
				     {}, 
				     "2*x+w==16 & w>= 5 & x>= 2");
    auto s_d = build_state({ "l3" }, 
				     {}, 
				     "2*x+w==5 & x>=0 & x <= 2");

    list<Symbolic_State> expected = { *s_a, *s_b , *s_c, *s_d};

    auto li = MODEL.get_all_states();
    //cout << "The states we got : " << endl;
    for (auto x : li) x->print();
    //cout << "The states we expected : " << endl;
    //for (auto x : expected) x.print();

    CHECK(compare_state_sets(li, expected));

}
