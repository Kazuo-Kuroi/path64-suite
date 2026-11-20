// Original allocator propagation, initializer-list and rollback coverage.
// UNSUPPORTED: c++98, c++03
#include <list>
#include <memory>
#include <iterator>
#include <cassert>
#include <new>
#include <utility>
#include <limits>
#include <stdexcept>
struct Allocation { void* pointer; int owner; std::size_t count; };
static Allocation blocks[1024];
static int outstanding, constructs, destroys;
template<class T, bool Copy, bool Move, bool Swap> struct Owner {
    typedef T value_type;
    typedef std::integral_constant<bool,Copy> propagate_on_container_copy_assignment;
    typedef std::integral_constant<bool,Move> propagate_on_container_move_assignment;
    typedef std::integral_constant<bool,Swap> propagate_on_container_swap;
    template<class U> struct rebind { typedef Owner<U,Copy,Move,Swap> other; };
    int id;
    explicit Owner(int n=1) noexcept : id(n) {}
    Owner(const Owner&) noexcept = default;
    Owner(Owner&& other) noexcept : id(other.id) { other.id=-1; }
    Owner& operator=(const Owner&) noexcept = default;
    Owner& operator=(Owner&& other) noexcept { id=other.id; other.id=-1; return *this; }
    template<class U> Owner(const Owner<U,Copy,Move,Swap>& other) noexcept : id(other.id) {}
    Owner select_on_container_copy_construction() const { return Owner(id+100); }
    T* allocate(std::size_t count) {
        assert(id>=-1);
        T* p=static_cast<T*>(::operator new(count*sizeof(T)));
        for (int i=0;i<1024;++i) if (!blocks[i].pointer) {
            blocks[i].pointer=p; blocks[i].owner=id; blocks[i].count=count; ++outstanding; return p;
        }
        assert(false); return 0;
    }
    void deallocate(T* p,std::size_t count) noexcept {
        for (int i=0;i<1024;++i) if (blocks[i].pointer==p) {
            assert(blocks[i].owner==id && blocks[i].count==count);
            blocks[i].pointer=0; --outstanding; ::operator delete(p); return;
        }
        assert(false);
    }
    template<class U,class... Args> void construct(U* p,Args&&... args) {
        ::new ((void*)p) U(std::forward<Args>(args)...); ++constructs;
    }
    template<class U> void destroy(U* p) noexcept { ++destroys; p->~U(); }
};
template<class T,class U,bool C,bool M,bool S>
bool operator==(const Owner<T,C,M,S>& a,const Owner<U,C,M,S>& b) { return a.id==b.id; }
template<class T,class U,bool C,bool M,bool S>
bool operator!=(const Owner<T,C,M,S>& a,const Owner<U,C,M,S>& b) { return !(a==b); }
struct Value {
    static int alive, budget;
    int n;
    explicit Value(int v=0) : n(v) { ++alive; }
    Value(const Value& other) : n(other.n) { check(); ++alive; }
    Value(Value&& other) : n(other.n) { check(); other.n=-1; ++alive; }
    Value& operator=(const Value& other) { check(); n=other.n; return *this; }
    static void check() { if (!budget) throw 8; if (budget>0) --budget; }
    ~Value() { --alive; }
};
int Value::alive;
int Value::budget=-1;
bool operator==(const Value& a,const Value& b) { return a.n==b.n; }
int main() {
    static_assert(std::is_nothrow_move_constructible<std::list<int>>::value,"list move");
    static_assert(std::is_nothrow_move_assignable<std::list<int>>::value,"list move assignment");

    try {
        std::list<int> too_large((std::numeric_limits<std::size_t>::max)());
        assert(false);
    } catch(const std::length_error&) { }

    {
        typedef Owner<int,true,true,true> A;
        std::list<int,A> source({1,2,3},A(4));
        std::list<int,A> copied(source);
        assert(copied.get_allocator().id==104 && copied.size()==3);
        std::list<int,A> explicit_copy(source,A(5));
        assert(explicit_copy.get_allocator().id==5);
        copied=source;
        assert(copied.get_allocator().id==4 && copied.front()==1);
        int* address=std::addressof(source.front());
        copied=std::move(source);
        assert(copied.get_allocator().id==4 && std::addressof(copied.front())==address);
        source.push_back(9);
        copied.swap(explicit_copy);
        assert(copied.get_allocator().id==5 && explicit_copy.get_allocator().id==4);
        copied={6,7}; copied.assign({8,9,10});
        auto inserted=copied.insert(copied.cbegin(),{11,12});
        assert(*inserted==11 && copied.size()==5);
        inserted=copied.insert(copied.cend(),0,99);
        assert(inserted==copied.end());
        auto next=copied.erase(copied.cbegin(),std::next(copied.cbegin(),2));
        assert(*next==8 && copied.size()==3);
        copied.assign(3,copied.front());
        assert(copied.size()==3 && copied.back()==8);
    }
    {
        typedef std::unique_ptr<int> P;
        typedef Owner<P,false,false,false> A;
        std::list<P,A> first(A(1)), second(A(2));
        first.emplace_back(new int(17)); second.emplace_back(new int(99));
        second=std::move(first);
        assert(second.get_allocator().id==2 && *second.front()==17);
        assert(first.get_allocator().id==1);
        std::list<P,A> extended(std::move(second),A(3));
        assert(extended.get_allocator().id==3 && *extended.front()==17);
        P* saved=std::addressof(extended.front());
        std::list<P,A> same(std::move(extended),A(3));
        assert(std::addressof(same.front())==saved && extended.empty());
        std::list<P,A> target(A(3));
        target=std::move(same);
        assert(std::addressof(target.front())==saved && same.empty());
        std::list<P,A> other(A(3));
        other.swap(target);
        assert(std::addressof(other.front())==saved && target.empty());
        target=std::move(other);
        target=std::move(target);
        assert(std::addressof(target.front())==saved);
        target.resize(4);
        assert(target.size()==4 && !target.back());
    }
    assert(outstanding==0 && constructs==destroys);
    {
        std::list<int> source={3,1,2};
        std::list<int> owner(std::move(source));
        source.clear(); source.sort(); source.reverse(); source.unique(); source.remove(7);
        assert(source.empty() && source.begin()==source.end());
        assert(source.insert(source.cend(),0,7)==source.end());
        int empty[1]={0};
        assert(source.insert(source.cend(),empty,empty)==source.end());
        source.splice(source.end(),owner,owner.begin());
        source.splice(source.end(),owner,owner.begin(),owner.end());
        assert(source.size()==3 && owner.empty());
        std::list<int> all(std::move(source));
        source.splice(source.end(),all);
        assert(source.size()==3 && all.empty());
        std::list<int> sorted(std::move(source)); sorted.sort();
        source.merge(sorted);
        assert(source.size()==3 && source.front()==1 && source.back()==3);
    }
    {
        typedef Owner<Value,false,false,false> A;
        std::list<Value,A> values(A(6));
        values.emplace_back(3);
        Value inputs[3]={Value(4),Value(5),Value(6)};
        const int allocations=outstanding;
        Value::budget=1;
        try { values.insert(values.cend(),inputs,inputs+3); assert(false); }
        catch(int n) { assert(n==8); }
        assert(values.size()==1 && values.front().n==3 && Value::alive==4);
        // Buffers may be retained after rollback, but contain no live values.
        assert(outstanding>=allocations);
        Value::budget=1;
        try { std::list<Value,A> failed(inputs,inputs+3,A(7)); assert(false); }
        catch(int n) { assert(n==8); }
        assert(Value::alive==4);
        Value::budget=0;
        try { values.assign(inputs,inputs+3); assert(false); }
        catch(int n) { assert(n==8); }
        assert(values.size()==1 && values.front().n==3);
        Value::budget=-1;
        values.remove(values.front());
    }
    assert(Value::alive==0 && outstanding==0 && constructs==destroys);
}
