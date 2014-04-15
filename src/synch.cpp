#include "synch.hpp"

using namespace std;

SynchBarrier::SynchBarrier(int n) : N(n)
{
}

void SynchBarrier::start()
{
    waitForAll();
    count = 0;
    st.notify_all();
}

void SynchBarrier::waitForAll()
{
    unique_lock<std::mutex> lck(mtx);

    while (count < N) mn.wait(lck);
}


void SynchBarrier::synch()
{
    unique_lock<std::mutex> lck(mtx);
    count++;
    if (count == N) mn.notify_one();
    st.wait(lck);
}

