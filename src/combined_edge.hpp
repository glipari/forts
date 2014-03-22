#ifndef __COMBINED_EDGE__
#define __COMBINED_EDGE__

#include  <vector>
#include  <list>

#include <automaton.hpp>

class Combined_edge {
public:
    std::vector<edge> edges;
    std::string sync_label;
    // The sync label set where the non empty sync_label is from
    std::vector<std::string> sync_set;

    Combined_edge();
    
    std::vector<Combined_edge> combine(const edge &e, const std::vector<std::string> e_syncs);
    bool operator == (const Combined_edge &ce) const;
};


#endif
