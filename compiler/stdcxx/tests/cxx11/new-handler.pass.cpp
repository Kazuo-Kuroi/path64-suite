// Original checks for synchronized handler access and allocator callbacks.
#include <new>
#include <type_traits>
#include <cassert>
#include <pthread.h>
#include <cstddef>

static int called;
static void a() { }
static void b() { }
static bool valid(std::new_handler h) { return !h || h == a || h == b; }
static void throwing_handler() {
    ++called;
    assert(std::get_new_handler() == throwing_handler);
    assert(std::set_new_handler(0) == throwing_handler);
    throw std::bad_alloc();
}
static void* worker(void*) {
    for (int i = 0; i < 10000; ++i) {
        assert(valid(std::set_new_handler(i % 2 ? a : b)));
        assert(valid(std::get_new_handler()));
    }
    return 0;
}
int main() {
    static_assert(noexcept(std::get_new_handler()), "query must not throw");
    static_assert(std::is_base_of<std::bad_alloc, std::bad_array_new_length>::value, "allocation error hierarchy");
    static_assert(std::is_nothrow_default_constructible<std::bad_array_new_length>::value, "nothrow default construction");
    static_assert(std::is_nothrow_copy_constructible<std::bad_array_new_length>::value, "nothrow copy construction");
    static_assert(std::is_nothrow_copy_assignable<std::bad_array_new_length>::value, "nothrow copy assignment");
    std::new_handler saved = std::set_new_handler(0);
    assert(!std::get_new_handler());
    assert(!std::set_new_handler(a) && std::get_new_handler() == a);
    assert(std::set_new_handler(b) == a && std::get_new_handler() == b);
    std::set_new_handler(0);
    pthread_t threads[4];
    for (int i = 0; i < 4; ++i) assert(!pthread_create(&threads[i], 0, worker, 0));
    for (int i = 0; i < 4; ++i) assert(!pthread_join(threads[i], 0));
    volatile std::size_t impossible = std::size_t(-1);
    std::set_new_handler(throwing_handler);
    try { void* p = ::operator new(impossible); ::operator delete(p); assert(false); }
    catch (const std::bad_alloc&) { }
    assert(called == 1 && !std::get_new_handler());
    std::set_new_handler(throwing_handler);
    assert(!::operator new(impossible, std::nothrow));
    assert(called == 2 && !std::get_new_handler());
    try { throw std::bad_array_new_length(); }
    catch (const std::bad_array_new_length& error) { assert(error.what() && *error.what()); }
    try { throw std::bad_array_new_length(); }
    catch (const std::bad_alloc& error) { assert(error.what() && *error.what()); }
    std::set_new_handler(saved);
}
