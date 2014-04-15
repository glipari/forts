#include <catch.hpp>
#include <synch.hpp>
#include <thread>
#include <vector>
#include <list>

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


TEST_CASE("split", "[parallel]")
{
    SECTION("split with vector, 1") {
	vector<int> v = {1, 2, 3};
	auto x = split(begin(v), end(v), v.size(), 1);
	REQUIRE(x.size() == 1);
	REQUIRE(*x[0].first == 1);
	REQUIRE(x[0].first == v.begin());
	REQUIRE(x[0].second == v.end());
    }
    SECTION("split with vector, 2") {
	vector<int> v = {1, 2, 3};
	auto x = split(begin(v), end(v), v.size(), 3);
	REQUIRE(x.size() == 3);
	REQUIRE(x[0].first == v.begin());
	REQUIRE(x[0].second == x[1].first);
	REQUIRE(x[1].second == x[2].first);
	REQUIRE(x[2].second == v.end());
	REQUIRE(*x[0].first == 1);
	REQUIRE(*x[1].first == 2);
	REQUIRE(*x[2].first == 3);
    }
    SECTION("split with vector, 3") {
	vector<int> v = {1, 2, 3};
	auto x = split(begin(v), end(v), v.size(), 2);
	REQUIRE(x.size() == 2);
	REQUIRE(x[0].first == v.begin());
	REQUIRE(x[0].second == x[1].first);
	REQUIRE(x[1].second == v.end());
	REQUIRE(*x[0].first == 1);
	REQUIRE(*x[1].first == 3);
    }
    SECTION("split with array, 3") {
	int v[] = {1, 2, 3};
	auto x = split(begin(v), end(v), 3, 2);
	REQUIRE(x.size() == 2);
	REQUIRE(x[0].first == begin(v));
	REQUIRE(x[0].second == x[1].first);
	REQUIRE(x[1].second == end(v));
	REQUIRE(*x[0].first == 1);
	REQUIRE(*x[1].first == 3);
    }
    SECTION("split with list, 3") {
	list<int> v = {1, 2, 3};
	auto x = split(begin(v), end(v), 3, 2);
	REQUIRE(x.size() == 2);
	REQUIRE(x[0].first == begin(v));
	REQUIRE(x[0].second == x[1].first);
	REQUIRE(x[1].second == end(v));
	REQUIRE(*x[0].first == 1);
	REQUIRE(*x[1].first == 3);
    }
    SECTION("split with list, 1") {
	list<int> v = {1, 2, 3};
	auto x = split(begin(v), end(v), 3, 1);
	REQUIRE(x.size() == 1);
	REQUIRE(*x[0].first == 1);
	REQUIRE(x[0].first == begin(v));
	REQUIRE(x[0].second == end(v));
	REQUIRE(*x[0].first == 1);
    }
    SECTION("split with list, over") {
	list<int> v = {1, 2};
	auto x = split(begin(v), end(v), 2, 5);
	REQUIRE(x.size() == 5);
	REQUIRE(*x[0].first == 1);
	REQUIRE(x[0].first  == begin(v));
	REQUIRE(x[0].second == x[1].first);
	REQUIRE(*x[1].first == 2);
	REQUIRE(x[1].second == end(v));
	REQUIRE(x[2].first  == end(v));
	REQUIRE(x[3].first  == end(v));
	REQUIRE(x[4].first  == end(v));
	REQUIRE(x[2].second == end(v));
	REQUIRE(x[3].second == end(v));
	REQUIRE(x[4].second == end(v));
    }
}
