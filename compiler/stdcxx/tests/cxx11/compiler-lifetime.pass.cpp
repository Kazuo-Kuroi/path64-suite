// Regression coverage for the Clang front end's C++ object lifetime lowering.
// Run with lcxx.py using this file's absolute path.
#include <cassert>
#include <initializer_list>

enum class Narrow : short { negative = -7, positive = 12 };

short narrow_value(Narrow value) { return static_cast<short>(value); }

struct HeapObject {
    int value;
    explicit HeapObject(int initial) : value(initial) { }
    void destroy() {
        assert(value == 42);
        delete this;
    }
};

struct Empty { };
struct alignas(16) AlignedEmpty { };

struct BranchException {
    BranchException() noexcept { }
    ~BranchException() noexcept { }
};

void throw_from_unbraced_branch(bool fail) {
    if (fail)
        throw BranchException();
}


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

struct AliasMember {
    typedef Owner Member;
    Member owner;
    explicit AliasMember(int* count) : owner(count) { }
};

Owner move_parameter(Owner value) {
    return static_cast<Owner&&>(value);
}

Owner forward_parameter(Owner value) {
    return move_parameter(static_cast<Owner&&>(value));
}

struct ThrowsOnConstruction {
    explicit ThrowsOnConstruction(const Owner&) { throw 7; }
};

Owner& static_owner() {
    static int count = 0;
    static Owner owner(&count);
    return owner;
}

Owner select_owner(bool first, int* left, int* right) {
    return first ? Owner(left) : Owner(right);
}

void consume(const Owner&) { }

static std::initializer_list<int> global_values = { 9, 12 };

int main() {
    assert(*static_owner().pointer == 1);
    (new HeapObject(42))->destroy();
    assert(narrow_value(Narrow::negative) == -7);
    assert(narrow_value(Narrow::positive) == 12);
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
    {
        AliasMember object(&owners);
        assert(owners == 1);
    }
    assert(owners == 0);
    try {
        ThrowsOnConstruction object{Owner(&owners)};
        assert(false);
    } catch (int value) {
        assert(value == 7);
        assert(owners == 0);
    }
    int other_owners = 0;
    {
        Owner selected = select_owner(true, &owners, &other_owners);
        assert(owners == 1);
        assert(other_owners == 0);
    }
    assert(owners == 0);
    {
        Owner selected = select_owner(false, &owners, &other_owners);
        assert(owners == 0);
        assert(other_owners == 1);
    }
    assert(other_owners == 0);
    bool take_branch = false;
    if (take_branch)
        consume(Owner(&owners));
    assert(owners == 0);
    take_branch = true;
    if (take_branch)
        consume(Owner(&owners));
    assert(owners == 0);
    for (int i = 0; i != 5; ++i)
        consume(Owner(&owners));
    assert(owners == 0);
    int iterations = 0;
    while (iterations++ != 5)
        consume(Owner(&owners));
    assert(owners == 0);
    iterations = 0;
    do
        consume(Owner(&owners));
    while (++iterations != 5);
    assert(owners == 0);
    bool caught_branch_exception = false;
    try { throw_from_unbraced_branch(true); }
    catch (...) { caught_branch_exception = true; }
    assert(caught_branch_exception);
    Empty empty_objects[4];
    Empty* empty_pointer = empty_objects;
    assert(++empty_pointer == empty_objects + 1);
    empty_pointer += 2;
    assert(empty_pointer - empty_objects == 3);
    assert(--empty_pointer == empty_objects + 2);
    AlignedEmpty aligned_objects[4];
    AlignedEmpty* aligned_pointer = aligned_objects;
    assert(++aligned_pointer == &aligned_objects[1]);
    aligned_pointer += 2;
    assert(aligned_pointer - aligned_objects == 3);
    assert(reinterpret_cast<char*>(aligned_pointer)
           - reinterpret_cast<char*>(aligned_objects) == 3 * sizeof(AlignedEmpty));
    assert(global_values.size() == 2);
    assert(global_values.begin()[0] == 9);
    assert(global_values.begin()[1] == 12);
}
