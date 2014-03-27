#ifndef __EDGE_HPP__
#define __EDGE_HPP__

#include <memory>
#include <vector>
#include <string>
#include <constraint.hpp>
#include <assignment.hpp>

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

    //std::shared_ptr<const constraint_node> guard;
    const std::string dest;
    const std::string sync_label;
    const constraint_node guard;
    std::vector<Assignment> assignments;

public:

    Edge(const std::string &destination, 
	 const std::string &label,
	 const constraint_node &guard_constraint,
	 const std::vector<Assignment> &ass_list);

    Linear_Constraint guard_to_Linear_Constraint(const CVList &cvl, const DVList &dvl) const;
    Linear_Constraint ass_to_Linear_Constraint(const CVList &cvl, const DVList &dvl) const;

    PPL::Variables_Set get_assignment_vars(const CVList &cvars) const;

    void print() const;

    int get_automaton_index() const { return a_index; }
    std::string get_dest() const { return dest; }
    std::string get_label() const { return sync_label; }
    int get_index() const { return index; }

    void set_automata_index(int a);


// for debugging and testing

    Assignment get_assignment_at(int i) const {
	return assignments.at(i);
    }

    constraint_node get_guard() const {
	return guard;
    }

};


#endif
