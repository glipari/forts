#ifndef __EDGE_FACTORY_HPP__
#define __EDGE_FACTORY_HPP__

#include <map>
#include <vector> 
#include <mutex> 
#include <combined_edge.hpp>
#include <model.hpp>

#define EDGE_FACTORY EdgeFactory::get_instance()

class EdgeFactory {
    static EdgeFactory *instance; 
//    std::map<Signature, std::vector<Combined_edge> > signature_to_combined_edges;    
    std::mutex mtx;

    EdgeFactory();
    EdgeFactory(const EdgeFactory& ef) = delete;
    EdgeFactory& operator=(const EdgeFactory& ef) = delete;
    
    
public:
    static EdgeFactory& get_instance();
    static void reset();

    std::vector<Combined_edge> get_edges(const Signature &s, 
					 const std::vector<Location *> &l);
};


#endif
