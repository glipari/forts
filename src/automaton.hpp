#ifndef _AUTOMATON_HPP_
#define _AUTOMATON_HPP_

#include <constraint.hpp>
#include <unique_index.hpp>

/**
   This class represents an edge in the automaton
*/
class edge {
public:
    /** Each edge is assigned a unique index. */
    int index;

    /** Automaton index */
    int a_index;

    std::shared_ptr<constraint_node> guard;
    std::vector<assignment> assignments;
    std::string sync_label;
    std::string dest;

    edge();

    Linear_Constraint guard_to_Linear_Constraint(const CVList &cvl, const DVList &dvl) const;
    Linear_Constraint ass_to_Linear_Constraint(const CVList &cvl, const DVList &dvl) const;
    void print();

    void set_automata_index(int a);
};


/** This class represent a location in the automaton  */
class location {
public:
    /** Automaton index */
    int a_index;

    bool bad = false;
    std::string name; 
    std::shared_ptr<constraint_node> invariant;
    std::vector<assignment> rates;
    std::vector<edge> outgoings;

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
