// Original move-only and duplicate-heavy selection regression.
// UNSUPPORTED: c++98, c++03
#include <algorithm>
#include <functional>
#include <vector>
#include <cassert>
#include <stdexcept>
struct Value {
    static int live;
    int key, id;
    Value(int k, int i): key(k), id(i) { ++live; }
    Value(Value&& x) noexcept: key(x.key), id(x.id) { x.id = -1; ++live; }
    Value& operator=(Value&& x) noexcept {
        assert(this != &x); key = x.key; id = x.id; x.id = -1; return *this;
    }
    Value(const Value&) = delete;
    Value& operator=(const Value&) = delete;
    ~Value() { --live; }
};
int Value::live;
struct Less {
    bool reverse;
    int* budget;
    bool operator()(const Value& a, const Value& b) const {
        assert(a.id >= 0 && b.id >= 0);
        if (budget && --*budget == 0) throw std::runtime_error("compare");
        return reverse ? a.key > b.key : a.key < b.key;
    }
};
void check(int n, int pattern, bool reverse) {
    int keys[65];
    for (int i = 0; i < n; ++i)
        keys[i] = pattern == 0 ? i : pattern == 1 ? n-i : pattern == 2 ? 7 :
                  pattern == 3 ? i%3 : (i*37+11)%67;
    int expected[65];
    std::copy(keys, keys+n, expected);
    if (reverse) std::sort(expected, expected+n, std::greater<int>());
    else std::sort(expected, expected+n);
    for (int nth = 0; nth <= n; ++nth) {
        std::vector<Value> a;
        for (int i = 0; i < n; ++i) a.emplace_back(keys[i], i);
        std::nth_element(a.begin(), a.begin()+nth, a.end(), Less{reverse, 0});
        bool seen[65] = {};
        for (int i = 0; i < n; ++i) {
            assert(a[i].id >= 0 && a[i].id < n && !seen[a[i].id]);
            seen[a[i].id] = true;
            assert(a[i].key == keys[a[i].id]);
        }
        if (nth == n) continue;
        assert(a[nth].key == expected[nth]);
        for (int i = 0; i < nth; ++i)
            for (int j = nth; j < n; ++j)
                assert((!Less{reverse, 0}(a[j], a[i])));
    }
}
struct Count {
    int* count;
    bool operator()(int a, int b) const { ++*count; return a < b; }
};
int main() {
    for (int n = 0; n <= 64; ++n)
        for (int pattern = 0; pattern != 5; ++pattern)
            for (int reverse = 0; reverse != 2; ++reverse)
                check(n, pattern, reverse != 0);
    assert(Value::live == 0);
    std::vector<int> same(4096, 9);
    int count = 0;
    std::nth_element(same.begin(), same.begin()+2048, same.end(), Count{&count});
    assert(count < 3*4096);
    int ints[] = {9, 2, 8, 3, 7, 4, 6, 5, 1, 0};
    std::nth_element(ints, ints+4, ints+10);
    assert(ints[4] == 4);
    for (int nth = 0; nth < 40; ++nth) {
        std::vector<bool> bits;
        for (int i = 0; i < 40; ++i) bits.push_back(i%3 == 0);
        std::nth_element(bits.begin(), bits.begin()+nth, bits.end());
        assert(bits[nth] == (nth >= 26));
        assert(std::count(bits.begin(), bits.end(), true) == 14);
    }
    for (int budget = 1; budget <= 50; ++budget) {
        std::vector<Value> a;
        for (int i = 0; i < 30; ++i) a.emplace_back((i*13)%31, i);
        int remaining = budget;
        try { std::nth_element(a.begin(), a.begin()+15, a.end(), Less{false, &remaining}); }
        catch (const std::runtime_error&) {}
        assert(Value::live == 30);
    }
    assert(Value::live == 0);
}
