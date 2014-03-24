#ifndef __EDGE_HPP__
#define __EDGE_HPP__

#include <memory>
#include <vector>
#include <string>
#include <constraint.hpp>
#include <assignment.hpp>

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
    std::vector<Assignment> assignments;
    std::string sync_label;
    std::string dest;

    edge();

    Linear_Constraint guard_to_Linear_Constraint(const CVList &cvl, const DVList &dvl) const;
    Linear_Constraint ass_to_Linear_Constraint(const CVList &cvl, const DVList &dvl) const;
    void print();

    void set_automata_index(int a);
};


#endif
