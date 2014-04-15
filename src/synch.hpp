#ifndef __BARRIER_HPP__
#define __BARRIER_HPP__

//#include <thread>
#include <mutex>    
#include <condition_variable>

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


#endif
