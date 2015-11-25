#ifndef __COMBINED_EDGE__
#define __COMBINED_EDGE__

#include  <vector>
#include  <list>

#include <automaton.hpp>

class Combined_edge {
    std::vector<Edge> edges;
    std::string sync_label;

    // The sync label set where the non empty sync_label is from
    std::vector<std::string> sync_set;

    std::vector<Location *> locations;

public:
    Combined_edge();

    Combined_edge(const Edge &e, const std::string &sync, 
		  const std::vector<std::string> &labels);

    Combined_edge(const std::vector<std::string> &labels);
    
    Combined_edge(const std::vector<Edge> &es, const std::string &sync, 
		  const std::vector<std::string> &labels);

    std::vector<Combined_edge> combine(const Edge &e, 
				       const std::vector<std::string> e_syncs);

    std::vector<Combined_edge> combine(const std::vector<std::string> new_labels);

    bool operator == (const Combined_edge &ce) const;

    const std::vector<Edge> & get_edges() const { return edges; }

    void set_locations(const std::vector<Location *>& locs);
    const std::vector<Location *>& get_locations() const;
};


#endif
