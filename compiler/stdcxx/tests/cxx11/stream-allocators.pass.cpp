// Original allocation ownership checks; no default allocator constructor.
// UNSUPPORTED: c++98, c++03
#include <sstream>
#include <memory>
#include <cassert>
#include <new>
struct Allocation { void* ptr; std::size_t count; int owner; };
static Allocation slots[2048];
static int live;
static bool fail_next;
template<class T, bool Propagate> struct Alloc : std::allocator<T> {
    typedef T value_type;
    typedef std::integral_constant<bool,Propagate> propagate_on_container_move_assignment;
    typedef std::integral_constant<bool,Propagate> propagate_on_container_swap;
    template<class U> struct rebind { typedef Alloc<U,Propagate> other; };
    int id;
    explicit Alloc(int n) : id(n) {}
    template<class U> Alloc(const Alloc<U,Propagate>& a) : id(a.id) {}
    T* allocate(std::size_t n) {
        if (fail_next) { fail_next=false; throw std::bad_alloc(); }
        void* p=::operator new(n*sizeof(T));
        for (unsigned i=0;i<2048;++i) if (!slots[i].ptr) {
            slots[i].ptr=p; slots[i].count=n; slots[i].owner=id; ++live;
            return static_cast<T*>(p);
        }
        assert(false); return 0;
    }
    void deallocate(T* p,std::size_t n) {
        assert(p);
        for (unsigned i=0;i<2048;++i) if (slots[i].ptr==p) {
            assert(slots[i].owner==id && slots[i].count==n);
            slots[i].ptr=0; --live; ::operator delete(p); return;
        }
        assert(false);
    }
    template<class U> bool operator==(const Alloc<U,Propagate>& a) const { return id==a.id; }
    template<class U> bool operator!=(const Alloc<U,Propagate>& a) const { return id!=a.id; }
};
template<bool P> void check() {
    typedef Alloc<char,P> A;
    typedef std::basic_string<char,std::char_traits<char>,A> String;
    typedef std::basic_stringbuf<char,std::char_traits<char>,A> Buffer;
    String one("abcdef",A(1)), two("xyz",A(2));
    Buffer source(one), dest(two);
    assert(source.str().get_allocator().id==1);
    source.pubseekpos(2,std::ios::in); source.pubseekpos(3,std::ios::out);
    dest=std::move(source);
    assert(dest.str().get_allocator().id==(P?1:2));
    assert(dest.str()==one && dest.sgetc()=='c');
    dest.sputc('!'); assert(dest.str()[3]=='!');
    Buffer moved(std::move(dest));
    assert(moved.str().get_allocator().id==(P?1:2));
    assert(moved.sgetc()=='c');
    Buffer equal(String("p",A(P?1:2)));
    equal=std::move(moved);
    assert(equal.str()[3]=='!');
    equal=std::move(equal); assert(equal.str()[3]=='!');
    Buffer left(one), right(P?two:one);
    left.swap(right);
    assert(left.str().get_allocator().id==(P?2:1));
    assert(right.str().get_allocator().id==1);
    char external[64];
    left.pubsetbuf(external,64);
    Buffer borrowed(std::move(left));
    assert(borrowed.str().get_allocator().id==(P?2:1));
    Buffer failure(one);
    char large[16384]; for (unsigned i=0;i<16384;++i) large[i]='g';
    fail_next=true;
    try { failure.sputn(large,16384); assert(false); } catch(const std::bad_alloc&) {}
    assert(failure.str()==one);
    failure.sputn(large,16384); assert(failure.str().size()==16384);
    String replacement(32768,'r',A(1));
    fail_next=true;
    try { failure.str(replacement); assert(false); } catch(const std::bad_alloc&) {}
    assert(failure.str().size()==16384 && failure.str()[0]=='g');
    if (!P) {
        Buffer from(one), to(two);
        fail_next=true;
        try { to=std::move(from); assert(false); } catch(const std::bad_alloc&) {}
        assert(to.str()==two && from.str()==one);
    }
}
int main() {
    check<false>(); check<true>();
    {
        typedef Alloc<wchar_t,false> A;
        typedef std::basic_string<wchar_t,std::char_traits<wchar_t>,A> String;
        typedef std::basic_stringbuf<wchar_t,std::char_traits<wchar_t>,A> Buffer;
        String text(L"wide",A(3));
        Buffer from(text), to(String(L"old",A(4)));
        from.pubseekpos(1,std::ios::in);
        to=std::move(from);
        assert(to.str()==text && to.str().get_allocator().id==4);
        assert(to.sgetc()==L'i');
        Buffer moved(std::move(to));
        assert(moved.str().get_allocator().id==4 && moved.sgetc()==L'i');
    }
    assert(live==0);
}
