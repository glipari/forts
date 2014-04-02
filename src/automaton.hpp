#ifndef _AUTOMATON_HPP_
#define _AUTOMATON_HPP_

#include <constraint.hpp>
#include <unique_index.hpp>
#include <edge.hpp>


/** This class represent a location in the automaton  */
class Location {
    /** Automaton index */
    int a_index;
    automaton *aut;

    bool bad = false;
    std::string name; 
    constraint_node invariant;
    std::vector<Assignment> rates;
    std::vector<Edge> outgoings;
public:
    // constructor
    Location(bool b, const std::string &n, 
	     const constraint_node &inv,
	     const std::vector<Assignment> &rt, 
	     const std::vector<Edge> &ed); 

    // used to set the index of the automaton that this location
    // belongs to.
    //void set_automata_index(int a);
    void set_automaton(automaton &a);
    automaton &get_automaton() { return *aut; }

    // the location name (local to the automaton)
    std::string get_name() const { return name; }
    // the set of outgoing edges
    std::vector<Edge> get_edges() const { return outgoings; }
    // the set of assignments for the rates
    std::vector<Assignment> get_rates() const { return rates; }
    // the location invariant
    constraint_node get_invariant() const { return invariant; }

    // returns the index of the automaton that this location belongs
    // to
    //int get_automaton_index() const { return a_index; }

    // returns true if this is a "bad" location 
    bool is_bad() const { return bad; }
    // sets the bad locations
    void set_bad(bool b) { bad = b; } 

    // prints the content of the automaton (for debugging)
    void print() const ;

    // returns the polyhedron that represents the rates
    Linear_Constraint rates_to_Linear_Constraint(const VariableList &cvl, const Valuations &dvl, VariableList& lvars) const;
    // returns the polyhedron that represent the invariant
    Linear_Constraint invariant_to_Linear_Constraint(const VariableList &cvl, const Valuations &dvl) const;
};

/** The class for an automaton  */
class automaton {
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

public:
    automaton(const std::string &n, 
	      const std::vector<std::string> &lbls,
	      const std::vector<Location> locs
	);
    
    void set_init_location(const std::string &init);
    void set_index(int i);

    std::string get_init_location() const { return init_loc_name; }
    int         get_index() const { return my_index; }
    std::string get_name() const { return name; }
    std::vector<std::string> get_labels() const { return labels; }
    Location&   get_location_by_name(std::string ln);

    std::vector<Location> get_all_locations() const { return locations; }
    void print() const;
    /** To check if there is inconsistency in the automaton. */
    bool      check_consistency(const VariableList &cvl, const Valuations &dvl) const;
};
#endif
