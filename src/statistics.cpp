#include "statistics.hpp"

TimeStatistic::TimeStatistic() : acc_time(0), max_time(0)
{
}

void TimeStatistic::start()
{
    last = std::chrono::system_clock::now();
}

void TimeStatistic::stop()
{
    time_t end = std::chrono::system_clock::now();
    acc_time += end - last;
    if (max_time < end - last) max_time = end - last;
    counter ++;
}

