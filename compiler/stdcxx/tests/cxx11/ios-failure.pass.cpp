// Original ABI, allocation-free copies and runtime-thrown stream failures.
// UNSUPPORTED: c++98, c++03
#include <sstream>
#include <system_error>
#include <type_traits>
#include <exception>
#include <cstring>
#include <cstdlib>
#include <new>
#include <cassert>
#include <pthread.h>
static bool deny_allocations;
void* operator new(std::size_t n) {
    if (deny_allocations) throw std::bad_alloc();
    void* p=std::malloc(n?n:1); if (!p) throw std::bad_alloc(); return p;
}
void* operator new[](std::size_t n) { return ::operator new(n); }
void operator delete(void* p) noexcept { std::free(p); }
void operator delete[](void* p) noexcept { std::free(p); }
#if __cplusplus >= 201402L
void operator delete(void* p,std::size_t) noexcept { std::free(p); }
void operator delete[](void* p,std::size_t) noexcept { std::free(p); }
#endif
extern "C" void legacy_stream_failure();
#ifndef FAILURE_CONSUMER_ONLY
extern "C" void legacy_stream_failure() {
    std::istringstream input("bad"); input.exceptions(std::ios::failbit);
    int value; input>>value;
}
#endif
#ifndef FAILURE_PROVIDER_ONLY
static std::exception_ptr captured;
static void* worker(void*) {
    try { legacy_stream_failure(); } catch(...) { captured=std::current_exception(); }
    return 0;
}
static void* copy_worker(void* raw) {
    const std::ios_base::failure& source=*static_cast<std::ios_base::failure*>(raw);
    for (int i=0;i<10000;++i) {
        std::ios_base::failure first(source), second(source);
        second=first;
        assert(std::strcmp(first.what(),source.what())==0);
        assert(second.code()==source.code());
    }
    return 0;
}
int main() {
    static_assert(std::is_base_of<std::system_error,std::ios_base::failure>::value,"failure hierarchy");
    static_assert(std::is_nothrow_copy_constructible<std::system_error>::value,"system error copy");
    static_assert(std::is_nothrow_copy_constructible<std::ios_base::failure>::value,"failure copy");
    static_assert(std::is_nothrow_copy_assignable<std::runtime_error>::value,"runtime assignment");
    std::ios_base::failure original("failure",std::make_error_code(std::errc::permission_denied));
    std::ios_base::failure assigned("assigned");
    std::runtime_error runtime("runtime"), runtime_assigned("other");
    deny_allocations=true;
    {
        std::ios_base::failure copy(original);
        assigned=copy;
        assert(copy.code()==original.code() && assigned.code()==original.code());
        assert(std::strcmp(copy.what(),original.what())==0);
        std::runtime_error other(runtime); runtime_assigned=other;
        assert(std::strcmp(runtime_assigned.what(),runtime.what())==0);
        try { throw copy; }
        catch(const std::system_error& e) { assert(e.code()==original.code()); }
    }
    // Shared message ownership must use atomic counters even in TLS builds.
    pthread_t copiers[4];
    for (int i=0;i<4;++i) assert(pthread_create(copiers+i,0,copy_worker,&original)==0);
    for (int i=0;i<4;++i) assert(pthread_join(copiers[i],0)==0);
    assert(std::strcmp(assigned.what(),original.what())==0);
    deny_allocations=false;
    try { legacy_stream_failure(); assert(false); }
    catch(const std::ios_base::failure& e) {
        assert(e.code()==std::make_error_code(std::io_errc::stream));
        assert(*e.what());
    }
    try { legacy_stream_failure(); assert(false); }
    catch(const std::system_error& e) { assert(e.code()==std::make_error_code(std::io_errc::stream)); }
    pthread_t thread;
    assert(pthread_create(&thread,0,worker,0)==0 && pthread_join(thread,0)==0);
    assert(captured);
    try { std::rethrow_exception(captured); assert(false); }
    catch(const std::ios_base::failure& e) { assert(*e.what() && e.code()==std::make_error_code(std::io_errc::stream)); }
    captured=nullptr;
}
#endif
