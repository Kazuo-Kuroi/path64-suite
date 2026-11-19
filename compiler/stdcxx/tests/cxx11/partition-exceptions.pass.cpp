// Original public-API checks under bounded temporary storage and exceptions.
#include <algorithm>
#include <cstdlib>
#include <cassert>
#include <cstddef>

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
struct predicate {
    int* calls;
    int* budget;
    bool operator()(const value& v) const {
        assert(v.id >= 0); ++*calls;
        if (*budget == 0) throw 2;
        if (*budget > 0) --*budget;
        return (v.key & 1) != 0;
    }
};
static void run(int move_limit, int pred_limit) {
    value a[] = {{0,0}, {1,1}, {2,2}, {3,3}, {4,4}, {5,5}, {6,6}, {7,7}};
    assert(value::alive == 8);
    int calls = 0;
    value::budget = move_limit;
    try {
        value* boundary = std::stable_partition(a, a+8, predicate{&calls, &pred_limit});
        assert(boundary == a+4 && calls == 8);
        const int ids[] = {1,3,5,7,0,2,4,6};
        for (int i = 0; i < 8; ++i) assert(a[i].id == ids[i]);
    }
    catch (int e) { assert(e == 1 || e == 2); }
    value::budget = -1;
    assert(value::alive == 8);
    assert(outstanding == 0);
}
int main() {
    const std::size_t limits[] = {0,1,2,3,4,8,64};
    for (unsigned c = 0; c < sizeof(limits)/sizeof(*limits); ++c) {
        capacity = limits[c];
        run(-1,-1); assert(value::alive == 0);
        for (int failure = 0; failure < 40; ++failure) {
            run(failure,-1); assert(value::alive == 0);
        }
        for (int failure = 0; failure < 8; ++failure) {
            run(-1,failure); assert(value::alive == 0);
        }
    }
}
