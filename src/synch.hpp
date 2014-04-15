#ifndef __BARRIER_HPP__
#define __BARRIER_HPP__

//#include <thread>
#include <mutex>    
#include <condition_variable>
#include <vector>
#include <cassert>
#include <cmath>

class SynchBarrier {
    std::condition_variable st;
    std::condition_variable mn;
    std::mutex mtx;
    const int N;
    int count = 0;
public:
    SynchBarrier(int n);
    // this is called by the manager to start everything
    void start();
    // this is called the manager to wait for everybody to sync
    void waitForAll();

    // this is called by every other thread to signal end of job
    void synch();
};

template<typename Iterator> 
std::vector<std::pair<Iterator, Iterator> > 
split(const Iterator &b, const Iterator &e, unsigned size, unsigned n) 
{
    assert(n >= 1 and size >= 1);
    const int x = std::ceil(double(size) / n);
    int assigned = 0;
    std::vector<std::pair<Iterator, Iterator> > v(n);

    v[0].first = b;
    for (int i=0; i<n-1; i++) {
	v[i].second = v[i].first;
	for (int j=0; j<x and assigned<size; ++j) {
	    ++(v[i].second);
	    assigned++;
	}
	v[i+1].first = v[i].second;
    }
    v[n-1].second = e;

    return v;
}


#endif
