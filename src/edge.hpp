#ifndef __EDGE_HPP__
#define __EDGE_HPP__

#include <memory>
#include <vector>
#include <string>
#include <constraint.hpp>
#include <assignment.hpp>

class automaton;
class Location;

/**
   This class represents an edge in the automaton
   
   TODO: what does it mean to copy an edge? what happens to 
   index and a_index? 
*/
class Edge {
    /** Each edge is assigned a unique index. */
    int index;

    /** Automaton index */
    int a_index;
    automaton *aut;
    Location *src_location;

    const std::string dest;
    Location *dst_location;

    const std::string sync_label;
    const constraint_node guard;
    std::vector<Assignment> assignments;

public:

    Edge(const std::string &destination, 
	 const std::string &label,
	 const constraint_node &guard_constraint,
	 const std::vector<Assignment> &ass_list);

    Linear_Constraint guard_to_Linear_Constraint(const VariableList &cvl, const Valuations &dvl) const;
    Linear_Constraint ass_to_Linear_Constraint(const VariableList &cvl, Valuations &dvl) const;

    PPL::Variables_Set get_assignment_vars(const VariableList &cvars) const;

    void print() const;

    int get_automaton_index() const { return a_index; }
    automaton & get_automaton() const { return *aut; }
    Location &get_src_location() const { return *src_location; }
    Location &get_dst_location() const { return *dst_location; }
    void set_src_location(Location &l);
    std::string get_dest() const { return dest; }
    std::string get_label() const { return sync_label; }
    int get_index() const { return index; }

    //void set_automata_index(int a);

// for debugging and testing

    Assignment get_assignment_at(int i) const {
	return assignments.at(i);
    }

    constraint_node get_guard() const {
	return guard;
    }

};


#endif
