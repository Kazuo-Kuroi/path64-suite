// Original regression: reference-argument materialization stays conditional.
#include <cassert>

static int calls;
int value() { ++calls; return 7; }
bool consume(const int& x) { return x == 7; }
int throwing_value() { ++calls; throw 42; }

struct source {
    int reads;
    int read() { ++reads; return 7; }
};

struct temporary {
    static int alive, constructed, destroyed, peak;
    static bool fail;
    temporary() { if (fail) throw 13; ++alive; ++constructed; if (alive > peak) peak = alive; }
    ~temporary() { --alive; ++destroyed; }
};
int temporary::alive, temporary::constructed, temporary::destroyed, temporary::peak;
bool temporary::fail;
bool use(const temporary&) { assert(temporary::alive > 0); return true; }
bool throw_after(const temporary&) { throw 9; }

int main() {
    volatile bool yes = true, no = false;
    calls = 0;
    assert(yes || consume(value()));
    assert(!(no && consume(value())));
    assert(calls == 0);
    assert(no || consume(value()));
    assert(yes && consume(value()));
    assert(calls == 2);
    assert(yes || consume(throwing_value()));
    assert(!(no && consume(throwing_value())));
    assert(calls == 2);
    try { (void)(no || consume(throwing_value())); assert(false); }
    catch (int x) { assert(x == 42); }
    assert(calls == 3);
    source s{0};
    source* p = 0;
    assert(!p || consume(p->read()));
    assert(!(p && consume(p->read())));
    p = &s;
    assert(p && consume(p->read()));
    assert(s.reads == 1);
    assert(no || (yes && consume(value())));
    assert(calls == 4);
    assert(yes || use(temporary()));
    assert(!(no && use(temporary())));
    assert(temporary::constructed == 0 && temporary::destroyed == 0);
    assert(no || (yes && use(temporary())));
    assert(temporary::alive == 0 && temporary::constructed == 1 && temporary::destroyed == 1);
    // All temporaries survive until the end of the full expression.
    assert(no || (use(temporary()) && use(temporary())));
    assert(temporary::alive == 0 && temporary::constructed == 3 && temporary::destroyed == 3);
    assert(temporary::peak == 2);
    try { (void)(no || throw_after(temporary())); assert(false); }
    catch (int x) { assert(x == 9); }
    assert(temporary::alive == 0 && temporary::constructed == 4 && temporary::destroyed == 4);
    assert(yes || (no || use(temporary())));
    assert(temporary::constructed == 4 && temporary::destroyed == 4);
    temporary::fail = true;
    try { (void)(no || use(temporary())); assert(false); }
    catch (int x) { assert(x == 13); }
    assert(temporary::alive == 0 && temporary::constructed == 4 && temporary::destroyed == 4);
}
