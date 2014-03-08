#include "expression.hpp"

// #include "syntax_trees.hpp"

using namespace std;
using namespace tipa;

bool expr_op_node::has_variable(const CVList &cvl) 
{
    return left->has_variable(cvl) || right->has_variable(cvl);
}

void expr_op_node::set_left(std::shared_ptr<expr_tree_node> l)
{
    left = l;
}

void expr_op_node::set_right(std::shared_ptr<expr_tree_node> r)
{
    right = r;
}


expr_var_node::expr_var_node(const std::string &n) : name(n) 
{}

int expr_var_node::eval(const DVList &dvl) 
{ 
    return var_2_val(name, dvl); 
}

bool expr_var_node::has_variable(const CVList &cvl) 
{
    return in_VList(name, cvl);
}

bool expr_var_node::check_linearity(const CVList &cvl) 
{
    return true;  
}

Linear_Expr expr_var_node::to_Linear_Expr(const CVList &cvl, const DVList &dvl) 
{
    Linear_Expr le;
    if ( in_VList(name, dvl) ) {
	le += var_2_val(name, dvl);
	return le;
    }
    PPL::Variable var = get_variable(name, cvl);
    le += var;
    return le;
}

expr_leaf_node::expr_leaf_node(int v) : value(v)
{}

int expr_leaf_node::eval(const DVList &dvl) 
{
    return value;
}

bool expr_leaf_node::has_variable(const CVList &cvl) 
{
    return false;
}

bool expr_leaf_node::check_linearity(const CVList &cvl) 
{
    return true;  
}

Linear_Expr expr_leaf_node::to_Linear_Expr(const CVList &cvl, const DVList &dvl) 
{
    Linear_Expr le;
    le += value;
    return le;
}


expr_builder::expr_builder () 
{}

void expr_builder::make_leaf(parser_context &pc)
{
    auto x = pc.collect_tokens();
    if (x.size() < 1) throw string("Error in collecting integer.");
    int v = atoi(x[x.size()-1].second.c_str());
    auto node = make_shared<expr_leaf_node>(v);
    st.push(node);
}
    
    
void expr_builder::make_var(parser_context &pc) 
{
    auto x = pc.collect_tokens();
    if (x.size() < 1) throw string("Error in collecting variable."); 
    string v = x[x.size()-1].second;
    auto node = make_shared<expr_var_node>(v);
    st.push(node);
}
    
int expr_builder::get_size() 
{
    return st.size();
}
    
shared_ptr<expr_tree_node> expr_builder::get_tree() 
{
    return st.top();
}

rule prepare_expr_rule(expr_builder &b)
{
    rule expr, primary, term, 
	op_plus, op_minus, op_mult, op_div,
	r_int, r_var; 

    expr = term >> *(op_plus | op_minus);
    op_plus = rule('+') > term;
    op_minus = rule('-') > term;

    term = primary >> *(op_mult | op_div);
    op_mult = rule('*') > primary;
    op_div = op_mult;

    primary = r_int | r_var | 
	rule('(') >> expr >> rule(')');

    r_int = rule(tk_int);
    r_var = rule(tk_ident);

    using namespace std::placeholders;

    r_var    [std::bind(&expr_builder::make_var,                   &b, _1)];
    r_int    [std::bind(&expr_builder::make_leaf,                  &b, _1)];
    op_plus  [std::bind(&expr_builder::make_op<plus_node>,         &b, _1)];
    op_minus [std::bind(&expr_builder::make_op<minus_node>,        &b, _1)];
    op_mult  [std::bind(&expr_builder::make_op<mult_node>,         &b, _1)];

    return expr;
}

shared_ptr<expr_tree_node> build_expression(const string &expr_input)
{
    expr_builder b;
    rule expr = prepare_expr_rule(b);

    stringstream str(expr_input);

    parser_context pc;
    pc.set_stream(str);

    bool f = false;
    try {
	f = expr.parse(pc);
    } catch (parse_exc &e) {
	cout << "Parse exception!" << endl;
    }

    if (!f) {
	cout << pc.get_formatted_err_msg();
    } else {
	auto tr = b.get_tree();
	return tr;
    }

    return nullptr;
}
