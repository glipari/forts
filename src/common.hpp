#ifndef _COMMON_HPP_
#define _COMMON_HPP_

#include <string>
#include <algorithm> 
#include <set>
#include <map>


// Helper function, to avoid calling the find explicitely.
template<class C, class X>
bool contains(const C &c, const X &x) 
{
    return std::find(c.begin(), c.end(), x) != c.end();
}

template<class X> 
bool contains(const std::set<X> &c, const X &x)
{
    return c.find(x) != c.end();
}

template<class X, class Y>
bool contains(const std::map<X, Y> &c, const X &x)
{
    return c.find(x) != c.end();
}

// list of variables, sorted by name
// actually a set. 
typedef std::set<std::string> VariableList; 

// Current valuations of the variables
typedef std::map<std::string, int> Valuations; 

int get_valuation(const Valuations &v, const std::string &n);

void set_valuation(Valuations &v, const std::string &n, int val);

// TO check if a string is an integer
bool an_integer(const std::string &s);

struct Parameter {
    std::string name;
    int min;
    int max;
    Parameter(std::string n, int mi, int ma);
//    Parameter();
};


// // A VARIABLE is a pair (variable name, integer valuation)
// // typedef pair<string, int> VARIABLE; 
// struct variable {
//     variable(const std::string &n) : name(n) {}
//     variable(const std::string &n, int v) : name(n), valuation(v) {}

//     std::string name;
//     int valuation;
//     void set_val(int v) { valuation = v;}
// };


// // The variable table for discrete variables
// typedef std::list<variable> DVList;
// // The continuous variable list
// typedef std::list<variable> CVList;
// // TODO ???? what is this? 
// typedef std::list<variable> VList;

// // returns the value associated to a discrete variable
// int var_2_val(const std::string &var, const DVList &dvl);

// // changes a value in the list
// void change_var_value(const std::string &var, DVList &dv, int val);

// // true is the variable is present in the list
// bool in_VList(const std::string &var, const VList &dvl);

#endif
