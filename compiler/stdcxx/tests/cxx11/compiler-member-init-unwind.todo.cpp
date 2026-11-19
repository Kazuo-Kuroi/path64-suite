// Open compiler regression: exceptions from a register-returned aggregate
// member initializer must propagate and destroy previously built members.
// Host Clang passes; psclang currently aborts during unwinding. Kept outside
// the .pass.cpp suite until constructor-initializer EH coverage is fixed.
#include <cassert>
struct guard {
    static int alive;
    guard() { ++alive; }
    ~guard() { --alive; }
};
int guard::alive;
struct result { long a, b; };
result fail() { throw 31; }
struct owner {
    guard first;
    result second;
    owner() : second(fail()) { assert(false); }
};
int main() {
    try { owner x; assert(false); }
    catch (int value) { assert(value == 31); }
    assert(guard::alive == 0);
}
