#include "ppl_adapt.hpp"

using namespace std;

PPL::Variable get_ppl_variable(const VariableList &cvl, const string &var) {
    int i = 0;
    for (auto const &x : cvl) {
	if (x == var)
	    return PPL::Variable(i);
	i++;
    }
    auto s = string("no such a variable \"") + var + string("\" in the CVList.");
    throw s;
}

using namespace PPL;
using namespace Parma_Polyhedra_Library::IO_Operators;

void time_elapse_assign(PPL::BD_Shape<int64_t> & bds, std::vector<int> &rates)
{
    bds.shortest_path_closure_assign();
    //cout << "Rates : " << endl;
    //for ( auto x : rates)
    //    cout << x << endl;
    //cout << "Before time elapse \n";
    //cout << "DBM : " << bds << endl;
    //cout << bds.dbm << endl;
    PPL::dimension_type num_dimensions = bds.space_dimension();
    for (dimension_type i = 1; i <= num_dimensions; ++i) {
        for (dimension_type j = 1; j <= num_dimensions; ++j) {
            if ( i == j) 
                continue;
            // if rates[i]==rates[j], then dbm[i][j] does not change
            if ( rates[i-1] == rates[j-1]) {

            }
            // else if rates[i]==1 and rates[j]==0, then dbm[i][j] <= does not change
            else if (rates[i-1]==1) {

            }
            // else if rates[i]==0 and rates[j]==1, then dbm[i][j] = infinity
            else {
                assign_r(bds.dbm[i][j], PLUS_INFINITY, ROUND_NOT_NEEDED);
            }
        }
    }
    for (dimension_type j = 1; j <= num_dimensions; ++j) {
        // Evaluate optimal upper bound for `x <= ub'.
        if ( rates[j-1] == 1) {
            assign_r(bds.dbm[0][j], PLUS_INFINITY, ROUND_NOT_NEEDED);
        }

    }

    bds.shortest_path_closure_assign();
    //cout << "After time elapse \n";
    //cout << "DBM : " << bds << endl;
    //cout << bds.dbm << endl;

}

void time_elapse_assign(PPL::Octagonal_Shape<int64_t> & oct, std::vector<int> &rates)
{
    oct.strong_closure_assign();
    PPL::dimension_type num_dimensions = oct.space_dimension();

    for (dimension_type i = 0; i < num_dimensions; ++i) {
        // 1) to evaluate 'x<=ub'
        if ( rates[i] == 1)
            assign_r(oct.matrix[2*i+1][2*i], PLUS_INFINITY, ROUND_NOT_NEEDED);

        // 2) to evaluate 'x+y<=ub'
        for (dimension_type j = 0; j < i; ++j)
            if ( rates[i] + rates[j] > 0)
                assign_r(oct.matrix[2*i+1][2*j], PLUS_INFINITY, ROUND_NOT_NEEDED);

        // 3) to evaluate 'x-y<=ub'
        for (dimension_type j = 0; j < num_dimensions; ++j) {
            if ( i == j)
                continue;
            if ( rates[i] > rates[j]) {
                if ( i < j)
                    assign_r(oct.matrix[2*j][2*i], PLUS_INFINITY, ROUND_NOT_NEEDED);
                else
                    assign_r(oct.matrix[2*i+1][2*j+1], PLUS_INFINITY, ROUND_NOT_NEEDED);
            }
        }

        // 4) to evaluate 'y-x<=ub'
        for (dimension_type j = 0; j < num_dimensions; ++j) {
            if ( i == j)
                continue;
            if ( rates[i] < rates[j]) {
                if ( i < j)
                    assign_r(oct.matrix[2*j][2*i], PLUS_INFINITY, ROUND_NOT_NEEDED);
                else
                    assign_r(oct.matrix[2*i+1][2*j+1], PLUS_INFINITY, ROUND_NOT_NEEDED);
            }
        }

        // 5) to evaluate '-x-y<=ub'
        for (dimension_type j = 0; j < i; ++j) {
            if ( (rates[i] == -1 && rates[j] != 1) || (rates[i] != 1 && rates[j] == -1))
                assign_r(oct.matrix[2*i][2*j+1], PLUS_INFINITY, ROUND_NOT_NEEDED);
        }

        // 6) to evaluate '-x<=ub'
        if ( rates[i] == -1)
            assign_r(oct.matrix[2*i][2*i+1], PLUS_INFINITY, ROUND_NOT_NEEDED);

    }

    oct.strong_closure_assign();
}
