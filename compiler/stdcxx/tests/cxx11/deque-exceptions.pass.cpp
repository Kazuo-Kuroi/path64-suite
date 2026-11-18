// Original regression coverage for segmented storage and failed construction.
#include <deque>
#include <cassert>

static int allocations;
static int allocations_before_throw = -1;
template <class T> struct CheckedAllocator {
    typedef T value_type;
    int id;
    explicit CheckedAllocator(int tag = 0) noexcept : id(tag) { }
    template <class U> CheckedAllocator(const CheckedAllocator<U>& rhs) noexcept : id(rhs.id) { }
    T* allocate(std::size_t n) {
        if (allocations_before_throw == 0) throw 7;
        if (allocations_before_throw > 0) --allocations_before_throw;
        T* p = std::allocator<T>().allocate(n);
        ++allocations;
        return p;
    }
    void deallocate(T* p, std::size_t n) {
        --allocations;
        std::allocator<T>().deallocate(p, n);
    }
};
template <class T, class U> bool operator==(const CheckedAllocator<T>& a, const CheckedAllocator<U>& b) { return a.id == b.id; }
template <class T, class U> bool operator!=(const CheckedAllocator<T>& a, const CheckedAllocator<U>& b) { return !(a == b); }

struct Element {
    static int alive;
    static int constructions_before_throw;
    int value;
    static void construct() {
        if (constructions_before_throw == 0) throw 8;
        if (constructions_before_throw > 0) --constructions_before_throw;
        ++alive;
    }
    explicit Element(int v = 0) : value(v) { construct(); }
    Element(const Element& rhs) : value(rhs.value) { construct(); }
    Element(Element&& rhs) noexcept : value(rhs.value) { rhs.value = -1; ++alive; }
    Element& operator=(Element&& rhs) noexcept { value = rhs.value; rhs.value = -1; return *this; }
    Element& operator=(const Element& rhs) { value = rhs.value; return *this; }
    ~Element() { --alive; }
};
int Element::alive;
int Element::constructions_before_throw = -1;

int main() {
    std::deque<double> counts(3.0, 7.0);
    assert(counts.size() == 3 && counts.front() == 7.0);
    counts.assign(5.0, 8.0);
    counts.insert(counts.cbegin() + 1, 2.0, 9.0);
    assert(counts.size() == 7 && counts[1] == 9.0 && counts[3] == 8.0);
    typedef std::deque<Element, CheckedAllocator<Element>> Deque;
    for (int front = 0; front != 2; ++front) {
        for (int count = 0; count != 2; ++count) {
            Deque values;
            allocations_before_throw = count;
            bool caught = false;
            try { if (front) values.emplace_front(9); else values.emplace_back(9); }
            catch (int e) { assert(e == 7); caught = true; }
            assert(caught && values.empty() && allocations == 0 && Element::alive == 0);
        }
        allocations_before_throw = -1;
        Deque values;
        Element::constructions_before_throw = 0;
        bool caught = false;
        try { if (front) values.emplace_front(9); else values.emplace_back(9); }
        catch (int e) { assert(e == 8); caught = true; }
        assert(caught && values.empty() && allocations == 0 && Element::alive == 0);
        Element::constructions_before_throw = -1;
    }
    {
        Deque values(CheckedAllocator<Element>(1));
        for (int i = 0; i != 2500; ++i) values.emplace_back(i);
        Element* stable = &values[1200];
        for (int i = 0; i != 2500; ++i) values.emplace_front(-i - 1);
        assert(stable == &values[3700] && stable->value == 1200);
        for (int i = 0; i != 2200; ++i) values.pop_front();
        for (int i = 0; i != 2200; ++i) values.pop_back();
        stable = &values[400];
        int stable_value = stable->value;
        allocations_before_throw = 0;
        values.shrink_to_fit();
        assert(stable == &values[400] && stable->value == stable_value);
        allocations_before_throw = -1;
        values.shrink_to_fit();
        assert(stable == &values[400] && stable->value == stable_value);
        values.emplace(values.cbegin() + 10, values.back());
        assert(values[10].value == values.back().value);
        Element source[37];
        std::size_t old_size = values.size();
        int old_alive = Element::alive;
        Element::constructions_before_throw = 5;
        bool caught = false;
        try { values.insert(values.cbegin() + 20, source, source + 37); }
        catch (int e) { assert(e == 8); caught = true; }
        assert(caught && values.size() == old_size && Element::alive == old_alive);
        Element::constructions_before_throw = 3;
        caught = false;
        try { values.resize(old_size + 10); }
        catch (int) { caught = true; }
        assert(caught && values.size() == old_size && Element::alive == old_alive);
        Element::constructions_before_throw = -1;
        Deque moved(std::move(values), CheckedAllocator<Element>(2));
        assert(moved.size() == old_size && moved.get_allocator().id == 2);
        Deque empty(CheckedAllocator<Element>(2));
        empty.swap(moved);
        assert(moved.empty() && empty.size() == old_size);
        moved.emplace_front(42);
        assert(moved.front().value == 42);
    }
    assert(allocations == 0 && Element::alive == 0);
}
