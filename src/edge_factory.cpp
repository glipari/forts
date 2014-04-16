#include <cassert>

#include "edge_factory.hpp"

using namespace std;

EdgeFactory* EdgeFactory::instance = nullptr;

EdgeFactory::EdgeFactory() 
{
}

EdgeFactory& EdgeFactory::get_instance()
{
    if (instance == nullptr) throw("Edge factory not initialized");
	// instance = new EdgeFactory();
    return *instance;
}


void EdgeFactory::reset()
{
    delete instance;
    instance = new EdgeFactory();
}

void combine(vector<Combined_edge> &edge_groups, const Location &l, 
	     const vector<string> new_labels,
	     bool first) 
{
    if (first) {
	vector<Edge> outgoings = l.get_edges();
	for (auto iit = outgoings.begin(); iit != outgoings.end(); iit++) {
            Combined_edge egroup(*iit, iit->get_label(), new_labels);
            // egroup.edges.push_back(*iit);
            // //if ( iit->sync_label != "") {
            //   egroup.sync_label = (iit->sync_label);
            //   egroup.sync_set = new_labels;
            // //}
            edge_groups.push_back(egroup);
	}
	return;
    } 
    vector<Combined_edge> copy = edge_groups;
    edge_groups.clear();
    // to combine every outgoing from "l" with every "edge group"
    vector<Edge> outgoings = l.get_edges();
    for ( auto &egroup : copy) {
      for ( auto it = outgoings.begin(); it != outgoings.end(); it++) {
        vector<Combined_edge> com = egroup.combine(*it, new_labels);
        for ( auto &eg : com)
          if ( not contains(edge_groups, eg))
            edge_groups.push_back(eg);
      }
    }
}


std::vector<Combined_edge> EdgeFactory::get_edges(const Signature &signature, 
						  const std::vector<Location *> &l)
{
    unique_lock<mutex> lck(mtx);
    
    auto it = signature_to_combined_edges.find(signature);
    if ( it != signature_to_combined_edges.end()) return it->second;
    else {
	vector<Combined_edge> edge_groups;
        bool first = true;
        for (auto p : l) {
            vector<string> new_labels = p->get_automaton().get_labels(); 
            combine(edge_groups, *p, new_labels, first);
	    first = false;
        }
	auto elem = pair<Signature, vector<Combined_edge> >(signature, edge_groups);
	auto result = signature_to_combined_edges.insert(elem);
	assert (result.second == true);
	return result.first->second;
    }
}
