#ifndef _AUTOMATON_HPP_
#define _AUTOMATON_HPP_

#include <constraint.hpp>

/**
   This class represents an edge in the automaton
*/
class edge {
public:
    std::shared_ptr<constraint_node> guard;
    std::vector<assignment> assignments;
    std::string sync_label;
    std::string dest;
    Linear_Constraint guard_to_Linear_Constraint(const CVList &cvl, const DVList &dvl) const;
    Linear_Constraint ass_to_Linear_Constraint(const CVList &cvl, const DVList &dvl) const;
    void print();
};


/** This class represent a location in the automaton  */
class location {
public:
    bool bad = false;
    std::string name; 
    std::shared_ptr<constraint_node> invariant;
    std::vector<assignment> rates;
    std::vector<edge> outgoings;
    void print();
    Linear_Constraint rates_to_Linear_Constraint(const CVList &cvl, const DVList &dvl);
    Linear_Constraint invariant_to_Linear_Constraint(const CVList &cvl, const DVList &dvl);
};

/** The class for an automaton  */
class automaton {
public:
    std::string name; 
    std::string init_loc_name;
    //std::shared_ptr<location> init_loc; 
    /** The set of synchronization labels */
    std::vector<std::string> labels;
    std::vector<location> locations;
    void print();
    /** To check if there is inconsistency in the automaton. */
    bool check_consistency(const CVList &cvl, const DVList &dvl);
    location &get_location(std::string ln);
};
#endif
