// Original C++98 regression: virtual redeclarations and one object evaluation.
#include <cassert>
static int calls, destroyed;
struct Base {
    virtual ~Base();
    virtual int value() const;
};
Base::~Base() {}
int Base::value() const { return 3; }
struct Derived : Base { ~Derived(); int value() const; };
Derived::~Derived() { ++destroyed; }
int Derived::value() const { return 9; }
const Base& reference(const Base& x) { ++calls; return x; }
Base* obtain(Base* p) { ++calls; return p; }
int main() {
    Derived d;
    Base* p=&d;
    assert(p->value()==9);
    assert(obtain(p)->value()==9 && calls==1);
    assert(reference(d).value()==9 && calls==2);
    assert(d.value()==9);
    assert(d.Base::value()==3);
    int (Base::*member)() const=&Base::value;
    assert((p->*member)()==9);
    Base* heap=new Derived;
    delete heap;
    assert(destroyed==1);
}
