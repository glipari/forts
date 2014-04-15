#ifndef __STATISTICS_HPP__
#define __STATISTICS_HPP__

#include <chrono>

class TimeStatistic {
    typedef std::chrono::time_point<std::chrono::high_resolution_clock> time_t;
    typedef std::chrono::duration<double> duration_t;

    duration_t acc_time ;
    duration_t max_time;
    time_t last;
    int counter = 0;
public:
    TimeStatistic();
    void start();
    void stop();

    double get_total() const { return acc_time.count(); }
    int get_counter() const { return counter; }
    double get_max() const { return max_time.count(); }
};





#endif
