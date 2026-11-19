// Original regression: evaluate prvalue member initializers at any offset.
#include <utility>
#include <cassert>

int calls;
struct aggregate { long first, second; };
aggregate make_aggregate(int n) { ++calls; return {n, n+1}; }
std::pair<long,long> make_pair(int n) { ++calls; return {n, n+1}; }
struct owner {
    aggregate first;
    long before;
    std::pair<long,long> second;
    long after;
    explicit owner(int n) : first(make_aggregate(n)), before(71),
        second(make_pair(n+2)), after(83) { }
    ~owner() { assert(before == 71 && after == 83); }
};
int main() {
    {
        owner x(5);
        assert(calls == 2);
        assert(x.first.first == 5 && x.first.second == 6);
        assert(x.second.first == 7 && x.second.second == 8);
        assert(x.before == 71 && x.after == 83);
    }
    const aggregate& bound = make_aggregate(19);
    assert(bound.first == 19 && bound.second == 20 && calls == 3);
    typedef int scalar;
    scalar values[4] = {};
    int remaining = 4;
    while (remaining) values[--remaining].~scalar();
    assert(remaining == 0);
    aggregate records[3] = {};
    remaining = 3;
    while (remaining) records[--remaining].~aggregate();
    assert(remaining == 0);
    make_aggregate(22).~aggregate();
    assert(calls == 4);
}
