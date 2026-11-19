// Regression: unwind completed members when a register-returned initializer throws.
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
