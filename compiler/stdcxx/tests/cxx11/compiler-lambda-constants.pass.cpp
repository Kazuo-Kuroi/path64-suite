// Original regression: omitted integral-constant captures have no stack slot.
// UNSUPPORTED: c++98, c++03
#include <limits>
#include <cassert>
int main() {
    const unsigned long long huge=(std::numeric_limits<unsigned long long>::max)();
    auto reference=[&] { return huge; };
    auto value=[=] { return huge; };
    auto empty=[] { return huge; };
    assert(reference()==huge && value()==huge && empty()==huge);
    const int negative=-197;
    auto signed_value=[] { return negative; }; assert(signed_value()==-197);
    const int x=13;
    auto address=[&] { return &x; }; assert(address()==&x);
    auto copy=[x] { return &x; }; assert(copy()!=&x && *copy()==13);
    auto local=[] { const int inside=29; const int* p=&inside; return *p; }; assert(local()==29);
    int changing=31; auto live=[&] { return changing; }; changing=37; assert(live()==37);
    auto nested=[] { const int outer=41; auto inner=[] { return outer; }; return inner(); }; assert(nested()==41);
}
