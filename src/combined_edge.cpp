#include "combined_edge.hpp"

using namespace std;

Combined_edge::Combined_edge(const Edge &e, const std::string &sync, 
			     const vector<string> &labels) :
    edges{e}, sync_label(sync), sync_set(labels)
{
}

bool Combined_edge::operator == (const Combined_edge &ce) const
{
    if (edges.size() != ce.edges.size())  return false;
    if (sync_label != ce.sync_label) return false;
    for (unsigned i = 0; i < edges.size(); i++)
	if ( edges[i].get_index() != ce.edges[i].get_index())
	    return false;
    if ( sync_set.size() != ce.sync_set.size())
      return false;
    for (unsigned i = 0; i < sync_set.size(); i++)
	if ( sync_set[i] != ce.sync_set[i])
	    return false;
    return true;
}

std::vector<Combined_edge> Combined_edge::combine(const Edge &e, 
						  const vector<string> new_labels)
{
    vector<string> common_labels;
    vector<string> union_labels;

    std::set_intersection(sync_set.begin(), 
                          sync_set.end(), 
                          new_labels.begin(),
                          new_labels.end(),
                          std::back_inserter(common_labels));
    std::set_union(sync_set.begin(), 
		   sync_set.end(), 
		   new_labels.begin(),
		   new_labels.end(),
		   std::back_inserter(union_labels));

    vector<Combined_edge> after_combination;

    // "this" can be triggered alone
    if (not contains(common_labels, sync_label)) {
	Combined_edge ce = *this;
	ce.sync_set = union_labels;
	after_combination.push_back(ce);
    }
    // "e" can be triggered alone
    if (not contains(common_labels, e.get_label())) {
	Combined_edge ce(e, e.get_label(), union_labels);
	// ce.edges.push_back(e);
	// ce.sync_label = e.sync_label;
	// ce.sync_set = union_labels;
	after_combination.push_back(ce);
    }
    // "this" and "e" need to synchronize 
    if (contains(common_labels, sync_label)) {
	if (e.get_label() == sync_label) {
	    Combined_edge ce = *this;
	    ce.edges.push_back(e);
	    ce.sync_set = union_labels;
	    after_combination.push_back(ce);
	}
    }

    return after_combination;
}
