// Original list move insertion, emplacement and failed-node reuse checks.
// UNSUPPORTED: c++98, c++03
#include <list>
#include <iterator>
#include <memory>
#include <cassert>
#include <utility>

struct Item {
    static int alive;
    static bool fail;
    int value;
    Item(int a, int b) : value(a+b) { if (fail) throw 9; ++alive; }
    Item(Item&& other) : value(other.value) {
        if (fail) throw 9;
        other.value=-1; ++alive;
    }
    Item(const Item&) = delete;
    Item& operator=(const Item&) = delete;
    ~Item() { --alive; }
    Item* operator&() { return 0; }
};
int Item::alive;
bool Item::fail;
struct Immobile {
    int value;
    explicit Immobile(int n) : value(n) {}
    Immobile(const Immobile&) = delete;
    Immobile(Immobile&&) = delete;
};
static int constructions;
static int allocations, allocator_moves;
static bool fail_allocation;
template<class T> struct Alloc : std::allocator<T> {
    template<class U> struct rebind { typedef Alloc<U> other; };
    int owner;
    Alloc() : owner(3) {}
    Alloc(const Alloc&) = default;
    Alloc& operator=(const Alloc&) = default;
    Alloc(Alloc&& other) noexcept : owner(other.owner) {
        other.owner=-1; ++allocator_moves;
    }
    template<class U> Alloc(const Alloc<U>& other) : owner(other.owner) {}
    T* allocate(std::size_t n, const void* hint=0) {
        assert(owner==3);
        if (fail_allocation) throw std::bad_alloc();
        T* p=std::allocator<T>::allocate(n,hint); ++allocations; return p;
    }
    void deallocate(T* p, std::size_t n) {
        assert(owner==3); --allocations; std::allocator<T>::deallocate(p,n);
    }
    template<class U, class... Args> void construct(U* p, Args&&... args) {
        ++constructions;
        ::new ((void*)p) U(std::forward<Args>(args)...);
    }
};
int main() {
    {
        std::list<Item, Alloc<Item> > values;
        values.emplace_back(1,2);
        Item* saved=std::addressof(values.front());
        values.emplace_front(4,5);
        std::list<Item, Alloc<Item> >::const_iterator position=values.cbegin();
        Item item(10,1);
        auto inserted=values.insert(position,std::move(item));
        assert(inserted->value==11 && item.value==-1);
        assert(saved==std::addressof(values.back()) && saved->value==3);
        std::front_inserter(values)=Item(20,1);
        std::back_inserter(values)=Item(30,1);
        std::inserter(values,values.begin())=Item(40,1);
        const std::size_t before=values.size();
        const int alive=Item::alive;
        Item::fail=true;
        try { values.emplace(values.cbegin(),50,1); assert(false); }
        catch(int n) { assert(n==9); }
        assert(values.size()==before && Item::alive==alive && values.front().value==41);
        try { values.push_back(std::move(item)); assert(false); }
        catch(int n) { assert(n==9); }
        assert(values.size()==before && Item::alive==alive);
        Item::fail=false;
        values.emplace_back(60,1);
        assert(values.back().value==61 && constructions>=9);
        const int before_move_allocations=allocations;
        fail_allocation=true;
        try {
            std::list<Item, Alloc<Item> > failed(std::move(values));
            assert(false);
        } catch(const std::bad_alloc&) { }
        fail_allocation=false;
        assert(allocations==before_move_allocations && values.size()==before+1);
        assert(saved->value==3);
        std::list<Item, Alloc<Item> > moved(std::move(values));
        assert(moved.size()==before+1 && saved->value==3 && allocator_moves==2);
        assert(values.empty());
        values.emplace_back(70,1);
        assert(values.front().value==71);
    }
    assert(Item::alive==0 && allocations==0);
    std::list<Immobile> fixed;
    fixed.emplace_front(7);
    fixed.emplace(fixed.cend(),8);
    assert(fixed.front().value==7 && fixed.back().value==8);
    std::list<int> copied;
    const int n=12;
    copied.insert(copied.cend(),n);
    copied.insert(copied.begin(),n);
    assert(copied.size()==2);
}
