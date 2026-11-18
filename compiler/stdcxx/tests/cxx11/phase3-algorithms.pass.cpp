// Original regressions for Phase 3 algorithm contracts.
#include <algorithm>
#include <iterator>
#include <numeric>
#include <cassert>

struct item { int value, id; };
struct less_item {
    int* calls;
    bool operator()(const item& a, const item& b) const {
        ++*calls;
        return a.value < b.value;
    }
};
struct forward_iterator : std::iterator<std::forward_iterator_tag, item> {
    item* p;
    explicit forward_iterator(item* q) : p(q) { }
    item& operator*() const { return *p; }
    forward_iterator& operator++() { ++p; return *this; }
    forward_iterator operator++(int) { auto old = *this; ++*this; return old; }
    bool operator==(forward_iterator b) const { return p == b.p; }
    bool operator!=(forward_iterator b) const { return p != b.p; }
};
struct equal_count {
    int* calls;
    bool operator()(int a, int b) const { ++*calls; return a == b; }
};
int main() {
    // Exhaustive small ternary sequences exercise ties, odd tails, and the
    // specified comparison bound through forward iterators.
    for (int n = 0; n <= 7; ++n) {
        int combinations = 1;
        for (int i = 0; i < n; ++i) combinations *= 3;
        for (int code = 0; code < combinations; ++code) {
            item a[7];
            int digits = code, lo = 0, hi = 0;
            for (int i = 0; i < n; ++i) {
                a[i] = item{digits % 3, i}; digits /= 3;
                if (a[i].value < a[lo].value) lo = i;
                if (a[i].value >= a[hi].value) hi = i;
            }
            int calls = 0;
            auto result = std::minmax_element(forward_iterator(a),
                forward_iterator(a + n), less_item{&calls});
            assert(result.first.p == a + (n ? lo : 0));
            assert(result.second.p == a + (n ? hi : 0));
            assert(calls <= (n ? 3 * (n - 1) / 2 : 0));
        }
    }
    int values[6];
    std::iota(values, values + 6, 4);
    int copied[6] = {};
    assert(std::copy_n(values, -2, copied) == copied);
    assert(std::copy_n(values, 6, copied) == copied + 6);
    assert(std::equal(values, values + 6, copied));
    assert(std::is_heap(values, values));
    assert(std::is_sorted(values, values + 6));
    int calls = 0;
    item a{2, 1}, b{2, 2};
    auto ties = std::minmax(a, b, less_item{&calls});
    assert(&ties.first == &a && &ties.second == &b && calls == 1);
#if __cplusplus >= 201402L
    constexpr int range[] = {4, 1, 4, 1};
    constexpr auto bounds = std::minmax({4, 1, 4, 1});
    static_assert(bounds.first == 1 && bounds.second == 4, "constexpr extrema");
    static_assert(std::min({4, 1, 3}) == 1, "constexpr minimum");
    static_assert(std::max({4, 1, 3}) == 4, "constexpr maximum");
    static_assert(std::min(4, 1) == 1 && std::max(4, 1) == 4, "constexpr binary extrema");
    static_assert(std::make_reverse_iterator(range + 4).base() == range + 4,
                  "constexpr reverse factory");
    calls = 0;
    assert(!std::equal(values, values + 6, copied, copied + 5, equal_count{&calls}));
    assert(calls == 0);
    auto mismatch = std::mismatch(values, values + 6, copied, copied + 3);
    assert(mismatch.first == values + 3 && mismatch.second == copied + 3);
#endif
}
