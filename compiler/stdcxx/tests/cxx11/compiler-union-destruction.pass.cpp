// Original union variants must only be destroyed by explicit user code.
// UNSUPPORTED: c++98, c++03
#include <new>
#include <cassert>
static int alive, destroyed, union_destructors;
struct Value {
    int id;
    explicit Value(int n) : id(n) { ++alive; }
    ~Value() { assert(alive>0); --alive; ++destroyed; }
};
union Named {
    Value value;
    int empty;
    Named() : empty(0) {}
    ~Named() { ++union_destructors; }
};
union Managed {
    Value value;
    int empty;
    Managed() : value(7) {}
    ~Managed() { value.~Value(); ++union_destructors; }
};
struct Anonymous {
    union { Value value; int empty; };
    bool active;
    explicit Anonymous(bool make_value) : empty(0),active(make_value) {
        if (active) ::new ((void*)&value) Value(9);
    }
    ~Anonymous() { if (active) value.~Value(); }
};
struct Enclosing {
    Named named;
    Value trailing;
    Enclosing() : trailing(11) {}
    ~Enclosing() {}
};
int main() {
    { Anonymous unused(false); }
    assert(alive==0 && destroyed==0);
    { Anonymous used(true); assert(alive==1); }
    assert(alive==0 && destroyed==1);
    {
        Named named;
        ::new ((void*)&named.value) Value(3);
        named.value.~Value();
    }
    assert(alive==0 && destroyed==2 && union_destructors==1);
    { Managed managed; assert(alive==1); }
    assert(alive==0 && destroyed==3 && union_destructors==2);
    { Enclosing enclosing; assert(alive==1); }
    assert(alive==0 && destroyed==4 && union_destructors==3);
}
