// Original constant vptr, base layout, bit-field and mixed-object regression.
// UNSUPPORTED: c++98, c++03
#include <cassert>
#include <cstdlib>
struct Left {
    int x;
    constexpr Left(int n=3) : x(n) {}
    virtual int left() const { return x; }
    virtual ~Left() {}
};
struct Right {
    int y;
    constexpr Right(int n=5) : y(n) {}
    virtual int right() const { return y; }
    virtual ~Right() {}
};
struct Both : Left, Right {
    unsigned bits:3;
    int z;
    constexpr Both(int n=7) : Left(3),Right(5),bits(6),z(n) {}
    int left() const { return x+z; }
    int right() const { return y+z; }
};
struct Nested { Both value; int tail; constexpr Nested() : value(9),tail(11) {} };
struct InheritedNested : Nested { constexpr InheritedNested() : Nested() {} };
inline Both& shared_polymorphic() { static Both value; return value; }
extern "C" Both* constant_vptr_provider();
#ifndef VPTR_CONSUMER_ONLY
extern "C" Both* constant_vptr_provider() { return &shared_polymorphic(); }
#endif
#ifndef VPTR_PROVIDER_ONLY
static Both global(12);
static Nested nested;
static InheritedNested inherited_nested;
// Priority forces a read before ordinary dynamic initialization. Constructors
// for constant-initialized objects must not subsequently reset mutations.
static int early;
static int destruction_order;
struct Counted : Left {
    int id;
    constexpr Counted(int n) : Left(n),id(n) {}
    ~Counted() { destruction_order=destruction_order*10+id; }
};
inline Counted* local_array() { static Counted values[2]={Counted(1),Counted(2)}; return values; }
void verify_destruction() { assert(destruction_order==21); }
struct Reader { Reader() {
    early=global.left()+inherited_nested.value.right(); global.z=13;
} };
__attribute__((init_priority(200))) static Reader reader;
int main() {
    assert(std::atexit(verify_destruction)==0);
    Counted* array=local_array(); assert(array[0].x==1 && array[1].x==2);
    assert(early==29 && global.z==13);
    Left* l=&global; Right* r=&global;
    assert(l->left()==16 && r->right()==18 && global.bits==6);
    l=&nested.value; r=&nested.value;
    assert(l->left()==12 && r->right()==14 && nested.tail==11);
    Both* p=constant_vptr_provider();
    assert(p==&shared_polymorphic());
    l=p; r=p; assert(l->left()==10 && r->right()==12 && p->bits==6);
    p->z=17; assert(shared_polymorphic().z==17);
}
#endif
