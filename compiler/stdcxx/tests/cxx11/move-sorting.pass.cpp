// Original regressions for sorting and heap contracts with move-only values.
#include <algorithm>
#include <vector>
#include <cassert>

struct value {
    int key, id;
    value(int k, int i) : key(k), id(i) { }
    value(value&& other) : key(other.key), id(other.id) { other.id = -1; }
    value& operator=(value&& other) {
        key = other.key; id = other.id; other.id = -1; return *this;
    }
    friend bool operator<(const value& a, const value& b) {
        assert(a.id >= 0 && b.id >= 0); return a.key < b.key;
    }
    value(const value&) = delete;
    value& operator=(const value&) = delete;
};
struct less {
    unsigned long* calls;
    bool operator()(const value& a, const value& b) const {
        assert(a.id >= 0 && b.id >= 0);
        ++*calls; return a.key < b.key;
    }
};
static void fill(std::vector<value>& values, int n, int pattern) {
    values.reserve(n);
    for (int i = 0; i < n; ++i) {
        int key = pattern == 0 ? i : pattern == 1 ? n-i :
                  pattern == 2 ? 7 : pattern == 3 ? i % 17 :
                  pattern == 4 ? (i < n/2 ? i : n-i) : (i * 73 + 19) % 127;
        values.emplace_back(key, i);
    }
}
static void permutation(const std::vector<value>& values) {
    std::vector<bool> seen(values.size(), false);
    for (unsigned i = 0; i < values.size(); ++i) {
        assert(values[i].id >= 0 && unsigned(values[i].id) < values.size());
        assert(!seen[values[i].id]); seen[values[i].id] = true;
    }
}
int main() {
    for (int pattern = 0; pattern < 6; ++pattern)
        for (int n = 0; n <= 2048; n = n < 32 ? n + 1 : n * 2) {
            std::vector<value> values;
            fill(values, n, pattern);
            unsigned long calls = 0;
            std::sort(values.begin(), values.end(), less{&calls});
            permutation(values);
            for (int i = 1; i < n; ++i) assert(values[i-1].key <= values[i].key);
            int bits = 1;
            for (int k = n; k > 1; k /= 2) ++bits;
            assert(calls <= 64UL * (n + 1) * bits);
        }
    for (int n = 0; n <= 40; ++n) {
        std::vector<value> values;
        fill(values, n, 5);
        unsigned long calls = 0;
        less comp{&calls};
        std::make_heap(values.begin(), values.end(), comp);
        assert(std::is_heap(values.begin(), values.end(), comp));
        for (int i = n; i > 0; --i) {
            std::pop_heap(values.begin(), values.begin() + i, comp);
            assert(std::is_heap(values.begin(), values.begin() + i - 1, comp));
        }
        permutation(values);
        for (int i = 1; i < n; ++i) assert(values[i-1].key <= values[i].key);
        for (int i = 1; i <= n; ++i) {
            std::push_heap(values.begin(), values.begin() + i, comp);
            assert(std::is_heap(values.begin(), values.begin() + i, comp));
        }
        std::sort_heap(values.begin(), values.end(), comp);
        permutation(values);
        for (int i = 1; i < n; ++i) assert(values[i-1].key <= values[i].key);
        for (int cut = 0; cut <= n; ++cut) {
            std::vector<value> selected;
            fill(selected, n, 5);
            std::partial_sort(selected.begin(), selected.begin() + cut, selected.end(), comp);
            permutation(selected);
            for (int i = 1; i < cut; ++i) assert(selected[i-1].key <= selected[i].key);
            for (int i = cut; cut && i < n; ++i) assert(selected[cut-1].key <= selected[i].key);
        }
    }
    {
        std::vector<value> values;
        fill(values, 128, 1);
        std::sort(values.begin(), values.end());
        std::make_heap(values.begin(), values.end());
        std::pop_heap(values.begin(), values.end());
        std::push_heap(values.begin(), values.end());
        std::sort_heap(values.begin(), values.end());
        std::partial_sort(values.begin(), values.begin() + 17, values.end());
        permutation(values);
        for (int i = 1; i < 128; ++i) assert(values[i-1].key <= values[i].key);
    }

    // Heap temporaries must hold values, rather than aliases to packed bits.
    for (int pattern = 0; pattern < 7; ++pattern) {
        std::vector<bool> bits(2048);
        int ones = 0;
        for (int i = 0; i < 2048; ++i) {
            bool bit = pattern == 0 || (i * 73 + pattern) % 13 > pattern;
            bits[i] = bit; ones += bit;
        }
        std::sort(bits.begin(), bits.end());
        for (int i = 0; i < 2048; ++i) assert(bits[i] == (i >= 2048 - ones));
        std::make_heap(bits.begin(), bits.end());
        std::sort_heap(bits.begin(), bits.end());
        for (int i = 0; i < 2048; ++i) assert(bits[i] == (i >= 2048 - ones));
    }

}
