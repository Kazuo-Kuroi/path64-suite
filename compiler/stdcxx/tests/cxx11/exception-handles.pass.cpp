// Original exception ownership, cross-thread propagation and catch cleanup checks.
// UNSUPPORTED: c++98, c++03
#include <exception>
#include <stdexcept>
#include <cstring>
#include <cassert>
#include <cstdlib>
#include <pthread.h>
#include <sys/wait.h>
#include <unistd.h>
static int alive, copies, order[64], used;
struct Value {
    int value;
    Value(int n): value(n) { ++alive; }
    Value(const Value& x): value(x.value) { ++alive; ++copies; }
    virtual ~Value() { --alive; }
};
struct Guard { int id; Guard(int n): id(n) {} ~Guard(){ order[used++] = id; } };
std::exception_ptr capture() {
    Guard outer(1);
    try { throw Value(42); }
    catch (...) { Guard inner(2); return std::current_exception(); }
}
void leave_void() { try { throw Value(7); } catch(...) { return; } }
int leave_value() { try { throw Value(8); } catch(const Value& v) { return v.value; } }
void* worker(void* data) {
    const std::exception_ptr& owner = *static_cast<std::exception_ptr*>(data);
    for (int i=0; i<100; ++i) {
        std::exception_ptr copy(owner);
        try { std::rethrow_exception(copy); }
        catch (const Value& v) { assert(v.value == 42 && std::current_exception() == owner); }
    }
    return 0;
}
struct FailCopy {
    static int count;
    FailCopy() {}
    FailCopy(const FailCopy&) { if (++count == 2) throw 99; }
};
int FailCopy::count;
struct BadCopy {
    BadCopy() {}
    BadCopy(BadCopy&&) noexcept {}
    BadCopy(const BadCopy&) { throw 71; }
};
static void terminate_now() { std::_Exit(86); }
int main() {
    assert(!std::current_exception());
    std::exception_ptr p = capture();
    assert(alive == 1 && used == 2 && order[0] == 2 && order[1] == 1);
    assert(!std::current_exception());
    pthread_t threads[4];
    for (int i=0; i<4; ++i) assert(pthread_create(&threads[i], 0, worker, &p) == 0);
    for (int i=0; i<4; ++i) assert(pthread_join(threads[i], 0) == 0);
    assert(alive == 1);
    p = nullptr;
    assert(alive == 0);
    leave_void(); assert(alive == 0 && !std::current_exception());
    assert(leave_value() == 8 && alive == 0);
    for (int i=0; i<3; ++i) {
        try { throw Value(i); }
        catch (Value v) {
            assert(alive == 2 && v.value == i);
            Guard guard(3);
            if (i < 2) continue;
            break;
        }
    }
    assert(alive == 0 && !std::current_exception());
    std::exception_ptr saved;
    try { throw Value(19); }
    catch (Value v) {
        saved = std::current_exception();
        v.value = 20;
        try { throw; } catch(const Value& original) { assert(original.value == 19); }
    }
    assert(alive == 1);
    try { std::rethrow_exception(saved); }
    catch(const Value& v) { assert(v.value == 19); saved = nullptr; }
    assert(alive == 0);
    try {
        try { throw 123; }
        catch(...) { std::throw_with_nested(std::runtime_error("outer failure")); }
    } catch(const std::exception& outer) {
        assert(std::strcmp(outer.what(), "outer failure") == 0);
        try { std::rethrow_if_nested(outer); assert(false); }
        catch(int n) { assert(n == 123); }
    }
    assert(!std::current_exception());
    std::exception_ptr failed_copy = std::make_exception_ptr(BadCopy());
    try { std::rethrow_exception(failed_copy); assert(false); }
    catch(int n) { assert(n == 71); }
    pid_t child = fork();
    assert(child >= 0);
    if (child == 0) {
        std::set_terminate(terminate_now);
        FailCopy x;
        try { try { throw x; } catch(FailCopy value) { std::_Exit(87); } }
        catch (...) { std::_Exit(88); }
        std::_Exit(89);
    }
    int status;
    assert(waitpid(child, &status, 0) == child);
    assert(WIFEXITED(status) && WEXITSTATUS(status) == 86);
    child = fork();
    assert(child >= 0);
    if (child == 0) {
        std::set_terminate(terminate_now);
        std::nested_exception empty;
        empty.rethrow_nested();
    }
    assert(waitpid(child, &status, 0) == child);
    assert(WIFEXITED(status) && WEXITSTATUS(status) == 86);
}
