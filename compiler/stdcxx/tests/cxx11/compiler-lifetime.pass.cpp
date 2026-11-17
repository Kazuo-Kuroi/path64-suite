// Regression coverage for the Clang front end's C++ object lifetime lowering.
// Run with lcxx.py using this file's absolute path.
#include <cassert>
#include <initializer_list>

struct Empty { };

struct EmptyBase : Empty {
    int* pointer;
    explicit EmptyBase(int* value) : Empty(), pointer(value) { }
};

struct ResetOnDestruction {
    int* value;
    explicit ResetOnDestruction(int* pointer) : value(pointer) { }
    ~ResetOnDestruction() { *value = 0; }
};

int value_before_cleanup() {
    int result = 42;
    ResetOnDestruction reset(&result);
    return result;
}

struct Owner {
    int* pointer;

    explicit Owner(int* value) : pointer(value) { ++*pointer; }
    Owner(const Owner& other) : pointer(other.pointer) { if (pointer) ++*pointer; }
    Owner(Owner&& other) : pointer(other.pointer) { other.pointer = 0; }
    ~Owner() { if (pointer) --*pointer; }
};

Owner move_parameter(Owner value) {
    return static_cast<Owner&&>(value);
}

Owner forward_parameter(Owner value) {
    return move_parameter(static_cast<Owner&&>(value));
}

static std::initializer_list<int> global_values = { 9, 12 };

int main() {
    int value = 3;
    EmptyBase object(&value);
    assert(object.pointer == &value);
    assert(value_before_cleanup() == 42);
    int owners = 0;
    {
        Owner first(&owners);
        Owner second = forward_parameter(first);
        assert(first.pointer == &owners);
        assert(second.pointer == &owners);
        assert(owners == 2);
    }
    assert(owners == 0);
    assert(global_values.size() == 2);
    assert(global_values.begin()[0] == 9);
    assert(global_values.begin()[1] == 12);
}
