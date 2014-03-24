#include "assignment_parser.hpp"

using namespace std;
using namespace tipa;

void assignment_builder::var_name(tipa::parser_context &pc) 
{
    auto v = pc.collect_tokens();
    var = v[v.size()-1].second;
}

Assignment assignment_builder::get_assignment() 
{
    auto expr = b.get_tree();
    return Assignment(var, expr);
}

Assignment build_assignment(const std::string &input)
{
    assignment_builder ab;
    rule ass = prepare_assignment_rule(ab);

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
	throw parse_exc("Error in parsing assignment : " + pc.get_formatted_err_msg()); 
    else 
	return ab.get_assignment();
}

rule prepare_assignment_rule(assignment_builder &ab)
{
    expr_builder &b = ab.b;

    rule expr = prepare_expr_rule(b);
    rule var_name = rule(tk_ident); 
    rule ass = var_name >> rule("'") >> rule("=") >> expr; 

    using namespace std::placeholders;
    var_name [std::bind(&assignment_builder::var_name, &ab, _1)];
    
    return ass;
}
