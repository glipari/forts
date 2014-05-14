#include "catch.hpp" 

#include <fstream>
#include <iostream>
#include <string>
#include <model.hpp>
#include <model_parser.hpp>
#include <constraint_parser.hpp>
#include <widened_sstate.hpp>

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


static shared_ptr<Symbolic_State> build_state(const std::vector<std::string> &locs, 
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
    auto res = make_shared<Widened_Symbolic_State> (locs, dv, cvx );
    res->widen();
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



TEST_CASE("Simple widened model", "[model][Space]")
{
    Model::reset();
    ifstream ifs("1t-1p.forts");
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

    MODEL.set_sstate_type(WIDENED);
    MODEL.SpaceExplorer();

    // build expected states

    auto s_a = build_state({ "idle" }, 
				     {{"C1",2}, {"D1",8}, {"T1",8}}, 
				     "c1==0 & p1 >= 0"); 
    auto s_b = build_state({ "x1R" }, 
				     {{"C1",2}, {"D1",8}, {"T1",8}}, 
				     "p1+c1==2 & c1 >=0 & c1 <= 2");

    list<Symbolic_State> expected = { *s_a, *s_b};



    auto li = MODEL.get_all_states();
    //cout << "The states we got : " << endl;
    for (auto x : li) x->print();

    CHECK(compare_state_sets(li, expected));

    const NNC_Polyhedron & x = s_a->get_cvx();
    const NNC_Polyhedron & y = s_b->get_cvx();


    PPL::NNC_Polyhedron xx(2);
    xx.add_constraint( PPL::Variable(0) <= 0);
    REQUIRE(xx.contains(x));
    REQUIRE(x.contains(xx));

    PPL::NNC_Polyhedron yy(2);
    yy.add_constraint( PPL::Variable(0) <= 2);
    yy.add_constraint( PPL::Variable(1) <= 2);
    yy.add_constraint( PPL::Variable(0) + PPL::Variable(1) <= 2);
    REQUIRE(yy.contains(y) );
    REQUIRE(y.contains(yy));


}
