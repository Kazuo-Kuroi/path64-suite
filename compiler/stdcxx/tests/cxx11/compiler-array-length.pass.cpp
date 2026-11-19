// Original compiler regression: reject invalid bounds before allocation.
// UNSUPPORTED: c++98, c++03
#include <new>
#include <cassert>
#include <cstdlib>
#include <cstddef>
#include <stdint.h>
static int allocations, constructors, evaluations;
static bool fail_allocation;
void* operator new[](std::size_t n) {
    ++allocations;
    if (fail_allocation) throw std::bad_alloc();
    void* p = std::malloc(n ? n : 1);
    if (!p) throw std::bad_alloc();
    return p;
}
void* operator new[](std::size_t n, const std::nothrow_t&) noexcept {
    ++allocations;
    return fail_allocation ? 0 : std::malloc(n ? n : 1);
}
void operator delete[](void* p) noexcept { std::free(p); }
struct Element {
    long data;
    Element() { ++constructors; }
    ~Element() { --constructors; }
};
long bound(long n) { ++evaluations; return n; }
int main() {
    volatile long negative = -1;
    try { int* p = new int[bound(negative)]; delete[] p; assert(false); }
    catch (const std::bad_array_new_length&) {}
    assert(allocations == 0 && evaluations == 1);
    volatile std::size_t huge = static_cast<std::size_t>(-1) / sizeof(int);
    try { int* p = new int[huge]; delete[] p; assert(false); }
    catch (const std::bad_array_new_length&) {}
    // The cookie must also fit inside the maximum object size.
    volatile std::size_t cookie_limit = PTRDIFF_MAX / sizeof(Element);
    try { Element* p = new Element[cookie_limit]; delete[] p; assert(false); }
    catch (const std::bad_array_new_length&) {}
    volatile int small = 1;
    try { int* p = new int[small]{1, 2}; delete[] p; assert(false); }
    catch (const std::bad_array_new_length&) {}
    assert(allocations == 0 && constructors == 0);
    assert(new (std::nothrow) Element[negative] == 0);
    assert(allocations == 0 && constructors == 0);
    volatile int count = 2;
    Element* p = new Element[count];
    assert(allocations == 1 && constructors == 2);
    delete[] p;
    assert(constructors == 0);
    fail_allocation = true;
    assert(new (std::nothrow) Element[count] == 0);
    assert(constructors == 0);
    assert(new (std::nothrow) int[count]() == 0);
    try { p = new Element[count]; assert(false); }
    catch (const std::bad_array_new_length&) { assert(false); }
    catch (const std::bad_alloc&) {}
    fail_allocation = false;
    int* empty = new int[0];
    delete[] empty;
}
