// Original C++98/11/14 catch-copy and handler-exit cleanup regression.
#include <exception>
#include <cassert>
#include <unistd.h>
#include <sys/wait.h>
static int alive, order[32], used;
struct Value {
    int value;
    Value(int n): value(n) { ++alive; }
    Value(const Value& x): value(x.value) { ++alive; }
    ~Value() { --alive; order[used++] = 3; }
};
struct Guard { int id; Guard(int n): id(n) {} ~Guard() { order[used++] = id; } };
int value_exit() {
    Guard outer(1);
    try { throw Value(42); }
    catch(Value v) { used = 0; assert(alive == 2); Guard inner(2); return v.value; }
}
void void_exit() {
    Guard outer(1);
    try { throw Value(7); }
    catch(const Value&) { used = 0; Guard inner(2); return; }
}
struct Fail {
    static int copies;
    Fail() {}
    Fail(const Fail&) { if (++copies == 2) throw 9; }
};
int Fail::copies;
static int arguments;
struct Argument { Argument(){++arguments;} ~Argument(){--arguments;} };
struct DefaultCopy {
    DefaultCopy() {}
    DefaultCopy(const DefaultCopy&, const Argument& = Argument()) { assert(arguments == 1); }
};
static bool throw_destructor;
struct ThrowingDestructor {
    bool copy;
    ThrowingDestructor(): copy(false) {}
    ThrowingDestructor(const ThrowingDestructor&): copy(true) {}
#if __cplusplus >= 201103L
    ~ThrowingDestructor() noexcept(false)
#else
    ~ThrowingDestructor()
#endif
    { if (copy && throw_destructor) { throw_destructor = false; throw 11; } }
};
void empty_handler() {
    Guard outer(4);
    try { throw ThrowingDestructor(); } catch(ThrowingDestructor) { }
}
void terminated() { ::_exit(86); }
int main() {
    assert(value_exit() == 42 && alive == 0 && used == 4);
    assert(order[0] == 2 && order[1] == 3 && order[2] == 3 && order[3] == 1);
    void_exit();
    assert(alive == 0 && used == 3);
    assert(order[0] == 2 && order[1] == 3 && order[2] == 1);
    try { throw Value(9); } catch(Value copy) { }
    assert(alive == 0);
    used = 0;
    for (int i=0; i<3; ++i) {
        try { throw Value(i); }
        catch (const Value&) { if (i < 2) continue; break; }
    }
    assert(alive == 0);
    try { throw DefaultCopy(); }
    catch(DefaultCopy value) { assert(arguments == 0); }
    assert(arguments == 0);
    used = 0;
    throw_destructor = true;
    try { empty_handler(); assert(false); }
    catch(int n) { assert(n == 11 && used == 1 && order[0] == 4); }
    pid_t child = fork();
    assert(child >= 0);
    if (!child) {
        std::set_terminate(terminated);
        Fail x;
        try { try { throw x; } catch(Fail copy) { ::_exit(87); } }
        catch(...) { ::_exit(88); }
    }
    int status;
    assert(waitpid(child, &status, 0) == child);
    assert(WIFEXITED(status) && WEXITSTATUS(status) == 86);
}
