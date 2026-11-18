// UNSUPPORTED: c++98, c++03, c++11
// Sized deallocation must delegate to user replacements of the unsized forms.
#include <new>
#include <cstdlib>
#include <cassert>

static unsigned scalar_deletes;
static unsigned array_deletes;
void operator delete(void* p) noexcept { ++scalar_deletes; std::free(p); }
void operator delete[](void* p) noexcept { ++array_deletes; std::free(p); }

int main() {
    void* scalar = ::operator new(23);
    ::operator delete(scalar, std::size_t(23));
    assert(scalar_deletes == 1 && array_deletes == 0);
    void* array = ::operator new[](47);
    ::operator delete[](array, std::size_t(47));
    assert(scalar_deletes == 1 && array_deletes == 1);
}
