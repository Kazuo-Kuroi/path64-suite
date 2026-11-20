// Original empty-base assignment must preserve overlapping derived storage.
#include <cassert>
#if __cplusplus >= 201103L
#include <utility>
#endif
struct Empty {};
struct Storage : Empty {
    volatile unsigned char bytes[16];
    Storage() { for (int i=0;i<16;++i) bytes[i]=0xa5; }
    void check() const { for (int i=0;i<16;++i) assert(bytes[i]==0xa5); }
};
static int left_calls, right_calls;
Empty& left(Storage& object) { ++left_calls; return object; }
const Empty& right(const Empty& object) { ++right_calls; return object; }
struct Effect {
    static int assignments;
    Effect& operator=(const Effect&) { ++assignments; return *this; }
};
int Effect::assignments;
int main() {
    Storage object;
    Empty source;
    Empty* result=&(left(object)=right(source));
    assert(result==static_cast<Empty*>(&object));
    assert(left_calls==1 && right_calls==1);
    object.check();
    result=&left(object).operator=(right(source));
    assert(result==static_cast<Empty*>(&object));
    assert(left_calls==2 && right_calls==2);
    object.check();
#if __cplusplus >= 201103L
    static_cast<Empty&>(object)=std::move(source);
    object.check();
#endif
    Effect a,b;
    a=b;
    assert(Effect::assignments==1);
}
