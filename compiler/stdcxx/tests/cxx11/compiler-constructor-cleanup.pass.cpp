// Original regressions for the completed subobjects of failed constructors.
#include <cassert>
static int alive, used, fail_id;
static int events[64];
struct guard {
    int id;
    explicit guard(int n) : id(n) { if (n == fail_id) throw 31; ++alive; }
    ~guard() { events[used++] = id; --alive; }
};
struct result { long a, b; };
result fail_result() { throw 31; }
int fail_scalar() { throw 31; }
static void reset(int fail = -1) { assert(alive == 0); used = 0; fail_id = fail; }
static void check(int n, int a = 0, int b = 0, int c = 0) {
    assert(alive == 0 && used == n);
    if (n > 0) assert(events[0] == a);
    if (n > 1) assert(events[1] == b);
    if (n > 2) assert(events[2] == c);
}
struct members {
    guard first, second;
    result data;
    members() : first(1), second(2), data(fail_result()) { assert(false); }
    ~members() { assert(false); }
};
struct scalar {
    guard first, second;
    int data;
    scalar() : first(1), second(2), data(fail_scalar()) { assert(false); }
};
struct body_failure {
    guard first, second;
    body_failure() : first(1), second(2) { guard local(3); throw 31; }
};
struct base { guard member; base() : member(10) { } };
struct derived : base {
    guard member;
    result data;
    derived() : member(11), data(fail_result()) { assert(false); }
    ~derived() { assert(false); }
};
struct local_catch {
    guard first, second;
    explicit local_catch(bool match) : first(1), second(2) {
        if (match) {
            try { guard local(3); throw 31; }
            catch (int n) { assert(n == 31 && alive == 2); }
        }
        else {
            try { guard local(3); throw 31; }
            catch (double) { assert(false); }
        }
    }
};
struct delegated {
    guard member;
    explicit delegated(int) : member(5) { }
    delegated() : delegated(0) { throw 31; }
};
struct referenced {
    guard& reference;
    guard member;
    result data;
    explicit referenced(guard& value) : reference(value), member(8), data(fail_result()) { assert(false); }
};
static int consume(const guard& value, bool throw_now) {
    assert(alive == 2 && value.id == 12);
    if (throw_now) throw 31;
    return value.id;
}
struct temporary_argument {
    guard first;
    int data;
    explicit temporary_argument(bool throw_now) : first(1), data(consume(guard(12), throw_now)) { assert(data == 12); }
};
template<class T> void fail() {
    try { T value; assert(false); }
    catch (int n) { assert(n == 31); }
}
int main() {
    reset(); fail<members>(); check(2,2,1);
    reset(); fail<scalar>(); check(2,2,1);
    reset(); fail<body_failure>(); check(3,3,2,1);
    reset(1); fail<members>(); check(0);
    reset(2); fail<members>(); check(1,1);
    reset(); fail<derived>(); check(2,11,10);
    reset(); { local_catch value(true); assert(alive == 2 && used == 1 && events[0] == 3); }
    check(3,3,2,1);
    reset();
    try { local_catch value(false); assert(false); }
    catch (int n) { assert(n == 31); }
    check(3,3,2,1);
    reset(); fail<delegated>(); check(1,5);
    reset();
    {
        guard external(7);
        try { referenced value(external); assert(false); }
        catch (int n) { assert(n == 31); }
        assert(alive == 1 && used == 1 && events[0] == 8);
    }
    check(2,8,7);

    reset();
    { temporary_argument value(false); assert(alive == 1 && used == 1 && events[0] == 12); }
    check(2,12,1);
    reset();
    try { temporary_argument value(true); assert(false); }
    catch (int n) { assert(n == 31); }
    check(2,12,1);
    reset(12);
    try { temporary_argument value(false); assert(false); }
    catch (int n) { assert(n == 31); }
    check(1,1);

}
