#ifndef _AUTOMATON_HPP_
#define _AUTOMATON_HPP_

#include <constraint.hpp>
#include <unique_index.hpp>
#include <edge.hpp>


/** This class represent a location in the automaton  */
class location {
public:
    /** Automaton index */
    int a_index;

    bool bad = false;
    std::string name; 
    constraint_node invariant;
    std::vector<Assignment> rates;
    std::vector<Edge> outgoings;

    location();

    void set_automata_index(int a);

    void print();
    Linear_Constraint rates_to_Linear_Constraint(const CVList &cvl, const DVList &dvl, CVList& lvars);
    Linear_Constraint invariant_to_Linear_Constraint(const CVList &cvl, const DVList &dvl);
};

/** The class for an automaton  */
class automaton {
public:
    /** my index, init to 0, then modified in the model */
    int my_index;
    /** The name of this automaton */
    std::string name;          
    /** the initial location */
    std::string init_loc_name;
    /** The set of synchronization labels */
    std::vector<std::string> labels;
    /** The set of locations */
    std::vector<location>    locations;


    automaton();

    void set_index(int i);

    void print();
    /** To check if there is inconsistency in the automaton. */
    bool     check_consistency(const CVList &cvl, const DVList &dvl);
    location &get_location(std::string ln);
};
#endif
