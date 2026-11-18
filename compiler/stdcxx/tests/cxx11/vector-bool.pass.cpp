// Original coverage for packed storage, allocation rollback and proxy swaps.
#include <vector>
#include <functional>
#include <cassert>

static bool fail_allocation;
template <class T> struct BitAllocator {
    typedef T value_type;
    BitAllocator() noexcept { }
    template <class U> BitAllocator(const BitAllocator<U>&) noexcept { }
    T* allocate(std::size_t n) {
        if (fail_allocation) throw 7;
        return std::allocator<T>().allocate(n);
    }
    void deallocate(T* p, std::size_t n) { std::allocator<T>().deallocate(p, n); }
};
template <class T, class U> bool operator==(BitAllocator<T>, BitAllocator<U>) { return true; }
template <class T, class U> bool operator!=(BitAllocator<T>, BitAllocator<U>) { return false; }

struct BitInput : std::iterator<std::input_iterator_tag, bool> {
    int* cursor;
    int limit;
    bool sentinel;
    BitInput(int& position, int count, bool end = false)
        : cursor(&position), limit(count), sentinel(end) { }
    bool operator*() const { return *cursor % 3 == 0; }
    BitInput& operator++() { ++*cursor; return *this; }
    struct PostIncrement {
        bool value;
        bool operator*() const { return value; }
    };
    PostIncrement operator++(int) {
        PostIncrement previous = {**this};
        ++*this;
        return previous;
    }
    bool operator==(const BitInput& rhs) const {
        return sentinel || rhs.sentinel ? *cursor == limit : cursor == rhs.cursor;
    }
    bool operator!=(const BitInput& rhs) const { return !(*this == rhs); }
};

int main() {
    int position = 0;
    std::vector<bool> bits(BitInput(position, 70), BitInput(position, 70, true));
    assert(position == 70 && bits.size() == 70);
    for (unsigned i = 0; i != 70; ++i) assert(bits[i] == (i % 3 == 0));
    position = 0;
    bits.assign(BitInput(position, 35), BitInput(position, 35, true));
    assert(position == 35 && bits.size() == 35);
    position = 0;
    bits.insert(bits.cbegin() + 10, BitInput(position, 40), BitInput(position, 40, true));
    assert(position == 40 && bits.size() == 75);
    for (unsigned i = 0; i != 40; ++i) assert(bits[10 + i] == (i % 3 == 0));
    for (unsigned i = 10; i != 35; ++i) assert(bits[40 + i] == (i % 3 == 0));
    bool value = true;
    bits[0] = false;
    static_assert(noexcept(bool(std::declval<std::vector<bool>::reference>())), "proxy conversion must not throw");
    static_assert(noexcept(std::declval<std::vector<bool>::reference&>() = true), "proxy assignment must not throw");
    static_assert(noexcept(bits.flip()), "bit flip must not throw");
    using std::swap;
    swap(bits[0], value);
    assert(bits[0] && !value);
    swap(value, bits[0]);
    assert(!bits[0] && value);

    bits.assign(70, false);
    bits.reserve(512);
    bits.flip();
    bits.resize(33);
    std::vector<bool> equal(33, true);
    assert(std::hash<std::vector<bool>>()(bits) == std::hash<std::vector<bool>>()(equal));
    bits.clear();
    equal.clear();
    equal.shrink_to_fit();
    assert(std::hash<std::vector<bool>>()(bits) == std::hash<std::vector<bool>>()(equal));

    typedef std::vector<bool, BitAllocator<bool>> Checked;
    Checked checked(65, true);
    checked.resize(checked.capacity(), true);
    std::size_t original_size = checked.size();
    fail_allocation = true;
    bool caught = false;
    try { checked.push_back(false); }
    catch (int e) { assert(e == 7); caught = true; }
    assert(caught && checked.size() == original_size);
    for (std::size_t i = 0; i != checked.size(); ++i) assert(checked[i]);
    fail_allocation = false;
    checked.reserve(512);
    original_size = checked.capacity();
    fail_allocation = true;
    checked.shrink_to_fit();
    assert(checked.capacity() == original_size);
    fail_allocation = false;
    checked.shrink_to_fit();
    assert(checked.capacity() < original_size);
    for (std::size_t i = 0; i != checked.size(); ++i) assert(checked[i]);
}
