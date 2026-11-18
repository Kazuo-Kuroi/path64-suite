// An exception must cross optimized helpers even without local handlers.
#include <cassert>

static int live_guards;
struct Guard {
    Guard() { ++live_guards; }
    ~Guard() { --live_guards; }
};
int* fail() { throw 7; }
typedef int* (*Callback)();
Callback volatile callback = fail;

int* bridge() {
    int* result = callback();
    return result + 1; // Keep a real call frame instead of a tail call.
}
void guarded_call() {
    Guard guard;
    bridge();
}
struct Parent {
    bool alive;
    Parent() : alive(true) { }
    ~Parent() { alive = false; }
};
struct Dependent {
    Parent& parent;
    explicit Dependent(Parent& p) : parent(p) { }
    ~Dependent() { assert(parent.alive); }
};
void branch_cleanup(bool first) {
    Parent outer;
    Dependent owner(outer);
    if (first) {
        Parent inner;
        Dependent child(inner);
        bridge();
    } else {
        Parent inner, other;
        Dependent child(inner), second(other);
        bridge();
    }
}

int main() {
    bool caught = false;
    try { guarded_call(); }
    catch (int value) { assert(value == 7); caught = true; }
    assert(caught && live_guards == 0);
    for (int first = 0; first != 2; ++first) {
        caught = false;
        try { branch_cleanup(bool(first)); }
        catch (int) { caught = true; }
        assert(caught);
    }
}
