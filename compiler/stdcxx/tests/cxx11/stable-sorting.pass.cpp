// Original move-only stable_sort checks with bounded storage and exceptions.
// UNSUPPORTED: c++98, c++03
#include <algorithm>
#include <cstdlib>
#include <cassert>
#include <cstddef>
#include <vector>

// Substitute the library's raw-buffer provider to exercise full, partial,
// and failed allocation deterministically; the public algorithm is unchanged.
static std::size_t capacity = 64;
static int outstanding;
namespace __rw {
extern "C" std::size_t __rw_tmpbuf(void** p, std::ptrdiff_t count, std::size_t size) {
    if (!count) {
        if (*p) { std::free(*p); --outstanding; }
        return 0;
    }
    std::size_t n = std::size_t(count) < capacity ? std::size_t(count) : capacity;
    *p = n ? std::malloc(n * size) : 0;
    if (*p) ++outstanding;
    return *p ? n : 0;
}
}
struct value {
    static int alive, budget;
    int key, id;
    value(int k, int i) : key(k), id(i) { ++alive; }
    static void move_check() { if (budget == 0) throw 1; if (budget > 0) --budget; }
    value(value&& other) {
        move_check(); key = other.key; id = other.id; other.id = -1; ++alive;
    }
    value& operator=(value&& other) {
        move_check();
        key = other.key; id = other.id; other.id = -1; return *this;
    }
    ~value() { --alive; }
    value(const value&) = delete;
    value& operator=(const value&) = delete;
};
int value::alive, value::budget = -1;
struct compare {
    int* calls;
    int* budget;
    bool reverse;
    bool operator()(const value& a, const value& b) const {
        assert(a.id >= 0 && b.id >= 0); ++*calls;
        if (*budget == 0) throw 2;
        if (*budget > 0) --*budget;
        return reverse ? a.key > b.key : a.key < b.key;
    }
};
static void run(int move_limit, int pred_limit) {
    value a[] = {{3,0}, {1,1}, {2,2}, {1,3}, {0,4}, {3,5}, {2,6}, {0,7}};
    assert(value::alive == 8);
    int calls = 0;
    value::budget = move_limit;
    try {
        std::stable_sort(a, a+8, compare{&calls, &pred_limit});
        const int ids[] = {4,7,1,3,2,6,0,5};
        for (int i = 0; i < 8; ++i) assert(a[i].id == ids[i]);
    }
    catch (int e) { assert(e == 1 || e == 2); }
    value::budget = -1;
    assert(value::alive == 8);
    assert(outstanding == 0);
}
static void larger(int n, int pattern, bool reverse) {
    std::vector<value> a;
    for (int i = 0; i < n; ++i)
        a.emplace_back(pattern == 0 ? i : pattern == 1 ? n-i :
                       pattern == 2 ? 5 : (i*37)%11, i);
    int calls = 0, budget = -1;
    std::stable_sort(a.begin(), a.end(), compare{&calls, &budget, reverse});
    bool seen[257] = {};
    for (int i = 0; i < n; ++i) {
        assert(a[i].id >= 0 && a[i].id < n && !seen[a[i].id]);
        seen[a[i].id] = true;
        const int id = a[i].id;
        assert(a[i].key == (pattern == 0 ? id : pattern == 1 ? n-id :
                            pattern == 2 ? 5 : (id*37)%11));
        if (i) {
            assert(reverse ? a[i-1].key >= a[i].key : a[i-1].key <= a[i].key);
            if (a[i-1].key == a[i].key) assert(a[i-1].id < a[i].id);
        }
    }
    int bits = 1;
    for (int k = n; k > 1; k /= 2) ++bits;
    assert(calls <= 4 * (n+1) * bits * bits);
    if (capacity >= std::size_t(n/2)) assert(calls <= 2 * (n+1) * bits);
    assert(outstanding == 0);
}
int main() {
    const std::size_t limits[] = {0,1,2,3,4,8,64,512};
    for (unsigned c = 0; c < sizeof(limits)/sizeof(*limits); ++c) {
        capacity = limits[c];
        for (int n = 0; n <= 257; n = n < 16 ? n+1 : n < 257 ? (std::min)(257, n*2+1) : 258)
            for (int pattern = 0; pattern != 4; ++pattern) {
                larger(n, pattern, false); assert(value::alive == 0);
                larger(n, pattern, true); assert(value::alive == 0);
            }
        run(-1,-1); assert(value::alive == 0);
        for (int failure = 0; failure < 100; ++failure) {
            run(failure,-1); assert(value::alive == 0);
        }
        for (int failure = 0; failure < 40; ++failure) {
            run(-1,failure); assert(value::alive == 0);
        }
    }
    std::vector<bool> bits;
    for (int i = 0; i < 97; ++i) bits.push_back(i%3 == 0);
    std::stable_sort(bits.begin(), bits.end());
    assert(std::count(bits.begin(), bits.end(), true) == 33);
    for (int i = 0; i < 97; ++i) assert(bits[i] == (i >= 64));
}
