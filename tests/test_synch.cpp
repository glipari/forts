#include <catch.hpp>
#include <synch.hpp>
#include <thread>
#include <vector>

using namespace std;

void worker(SynchBarrier &b, int &elem)
{
    elem = 0;
    b.synch();
    elem++;
    b.synch();
    elem *= 2;
    b.synch();
}


TEST_CASE("barrier", "[parallel]")
{
    SynchBarrier barrier(4);
    vector<int> elems = {1, 2, 3, 4};
    vector<thread> th;
    th.push_back(thread(worker, ref(barrier), ref(elems[0])));
    th.push_back(thread(worker, ref(barrier), ref(elems[1])));
    th.push_back(thread(worker, ref(barrier), ref(elems[2])));
    th.push_back(thread(worker, ref(barrier), ref(elems[3])));

    cout << "Starting the parallel test with barriers" << endl;

    vector<int> e1 = {0, 0, 0, 0};
    vector<int> e2 = {1, 1, 1, 1};
    vector<int> e3 = {2, 2, 2, 2};
    
    barrier.waitForAll();

    cout << "All threads synchronized on the first synch point" << endl;

    REQUIRE(elems == e1);
    barrier.start();
    
    cout << "All threads synchronized on the first synch point" << endl;
    
    barrier.waitForAll();
    REQUIRE(elems == e2);
    barrier.start();
    barrier.waitForAll();
    REQUIRE(elems == e3);
    barrier.start();
    for (auto &x : th) x.join();
    CHECK(true);
}
