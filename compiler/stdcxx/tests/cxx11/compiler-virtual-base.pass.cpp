// Regression coverage for construction VTTs, virtual-base adjustment and
// interoperability with the C++98 stream runtime.
#include <cassert>
#include <sstream>
#include <string>
#include <vector>

struct VirtualBase {
    static int alive;
    int value;

    VirtualBase() : value(17) { ++alive; }
    virtual int identity() const { return 1; }
    virtual ~VirtualBase() {
        assert(identity() == 1);
        --alive;
    }
};
int VirtualBase::alive = 0;

struct Left : virtual VirtualBase {
    int left;
    Left() : left(2) {
        assert(value == 17);
        assert(identity() == 2);
    }
    int identity() const { return 2; }
    ~Left() {
        assert(value == 17);
        assert(identity() == 2);
    }
};

struct Right : virtual VirtualBase {
    int right;
    Right() : right(3) {
        assert(value == 17);
        assert(identity() == 3);
    }
    int identity() const { return 3; }
    ~Right() {
        assert(value == 17);
        assert(identity() == 3);
    }
};

struct Diamond : Left, Right {
    long payload;
    Diamond() : payload(42) {
        assert(value == 17);
        assert(identity() == 4);
    }
    int identity() const { return 4; }
    ~Diamond() { assert(identity() == 4); }
};

struct Further : Diamond {
    char extra[16];
    int identity() const { return 5; }
};

Left* get_left(Diamond* object, int* calls) {
    ++*calls;
    return object;
}

int main() {
    {
        Further object;
        assert(VirtualBase::alive == 1);
        Left* left = &object;
        Right* right = &object;
        VirtualBase* from_left = left;
        VirtualBase* from_right = right;
        assert(from_left == from_right);
        assert(from_left->value == 17);
        assert(from_left->identity() == 5);
        assert(static_cast<VirtualBase&>(*left).value == 17);
        int calls = 0;
        VirtualBase* from_call = get_left(&object, &calls);
        assert(calls == 1);
        assert(from_call == from_left);
    }
    assert(VirtualBase::alive == 0);
    Diamond* empty = 0;
    VirtualBase* null_base = empty;
    assert(null_base == 0);

    std::vector<bool> bits;
    bits.push_back(false);
    bits.push_back(true);
    assert(bits.cend() - bits.cbegin() == 2);
    assert(!*bits.cbegin() && *bits.crbegin());
    assert(bits.crbegin() != bits.crend());

    std::ostringstream stream;
    assert(stream.str().empty());
    stream << "value " << 17;
    assert(stream.str() == "value 17");
}
