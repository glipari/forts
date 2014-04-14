#ifndef __STATISTICS_HPP__
#define __STATISTICS_HPP__

#include <chrono>

class TimeStatistic {
public:
    typedef std::chrono::time_point<std::chrono::high_resolution_clock> time_t;
    typedef std::chrono::duration<double> duration_t;
private:
    duration_t acc_time ;
    duration_t max_time;
    time_t last;
    int counter = 0;
    
public:
    TimeStatistic();
    void start();
    void stop();

    duration_t get_total() const { return acc_time; }
    int get_counter() const { return counter; }
};


#endif
