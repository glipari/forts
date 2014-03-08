#include "assignment.hpp"

using namespace std;
using namespace tipa;

assignment build_assignment(const std::string &input)
{
    assignment a;
    expr_builder b;

    rule expr = prepare_expr_rule(b);
    rule var_name = rule(tk_ident); 
    rule ass = var_name >> rule("'") >> rule("=") >> expr; 

    auto fun = [&a](parser_context &pc) {
	auto v = pc.collect_tokens();
	a.x = v[v.size() - 1].second;
    };

    var_name[fun];

    stringstream str(input);

    parser_context pc;
    pc.set_stream(str);

    bool flag = false;
    try {
	flag = ass.parse(pc);
    } catch (parse_exc &e) {
	cout << "Parse exception!" << endl;
    }

    if (!flag) 
	throw string("Error in parsing assignment: ") + pc.get_formatted_err_msg(); 
    else {
	a.expr = b.get_tree();
	return a;
    }
    
}
