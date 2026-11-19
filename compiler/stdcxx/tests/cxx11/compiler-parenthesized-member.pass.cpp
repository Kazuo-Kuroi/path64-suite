// Original C++98-compatible regression for parenthesized member callees.
#include <cassert>
struct Object {
    int value;
    explicit Object(int n) : value(n) {}
    int add(int n) { value+=n; return value; }
    int get() const { return value; }
};
static int calls;
Object* obtain(Object& x) { ++calls; return &x; }
int main() {
    Object x(3);
    assert((x.add)(4)==7);
    assert(((x.add))(2)==9);
    assert((obtain(x)->add)(1)==10 && calls==1);
    const Object& c=x;
    assert((c.get)()==10);
}
