#ifndef _AUTOMATON_HPP_
#define _AUTOMATON_HPP_

#include <constraint.hpp>
#include <unique_index.hpp>
#include <edge.hpp>


/** This class represent a location in the automaton  */
class Location {
    /** Automaton index */
    int a_index;

    bool bad = false;
    std::string name; 
    constraint_node invariant;
    std::vector<Assignment> rates;
    std::vector<Edge> outgoings;

    //location();

public:

    Location(bool b, const std::string &n, 
	     const constraint_node &inv,
	     const std::vector<Assignment> &rt, 
	     const std::vector<Edge> &ed); 

    void set_automata_index(int a);

    std::string get_name() const { return name; }
    std::vector<Edge> get_edges() const { return outgoings; }
    std::vector<Assignment> get_rates() const { return rates; }
    constraint_node get_invariant() const { return invariant; }
    int get_automaton_index() const { return a_index; }
    bool is_bad() const { return bad; }
    void set_bad(bool b) { bad = b; } 

    void print() const ;
    Linear_Constraint rates_to_Linear_Constraint(const CVList &cvl, const DVList &dvl, CVList& lvars) const;
    Linear_Constraint invariant_to_Linear_Constraint(const CVList &cvl, const DVList &dvl) const;
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
    std::vector<Location>    locations;


    automaton();

    std::string get_name() const { return name; }

    void set_index(int i);

    void print();
    /** To check if there is inconsistency in the automaton. */
    bool     check_consistency(const CVList &cvl, const DVList &dvl);
    Location &get_location(std::string ln);
};
#endif
