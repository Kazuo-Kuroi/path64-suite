// Original C++11 stream state, buffer ownership and position regressions.
// UNSUPPORTED: c++98, c++03
#include <sstream>
#include <utility>
#include <type_traits>
#include <cassert>
static int events;
void callback(std::ios_base::event e, std::ios_base&, int) {
    if (e == std::ios_base::erase_event) ++events;
}
template<class C> void check() {
    typedef std::basic_string<C> String;
    typedef std::basic_stringstream<C> Stream;
    const C text[] = {C('1'),C('2'),C('3'),C('4'),C('5'),0};
    Stream a(text);
    a.seekp(2); a.seekg(1);
    a.get(); assert(a.gcount()==1);
    a.precision(3); a.width(7); a.fill(C('#'));
    int index=std::ios_base::xalloc();
    int marker=0;
    a.iword(index)=91; a.pword(index)=&marker;
    a.register_callback(callback,0);
    a.tie(&a);
    // A self tie would recurse during actual I/O; moving only transfers it.
    Stream b(std::move(a));
    assert(a.tie()==0 && b.tie()==&a);
    b.tie(0);
    assert(b.gcount()==1 && a.gcount()==0);
    assert(b.iword(index)==91 && b.pword(index)==&marker);
    assert(b.precision()==3 && b.width()==7 && b.fill()==C('#'));
    assert(b.tellg()==2 && b.tellp()==2);
    b.width(0); b.put(C('X'));
    assert(b.str()[2]==C('X'));
    a.clear(); a.str(String(1,C('a'))); a.put(C('z'));
    assert(b.str()[2]==C('X'));
    Stream c(String(1,C('c')));
    auto* cb=c.rdbuf(); auto* bb=b.rdbuf();
    c.swap(b);
    assert(c.rdbuf()==cb && b.rdbuf()==bb);
    assert(c.str()[2]==C('X') && b.str()==String(1,C('c')));
    assert(c.gcount()==1 && b.gcount()==0);
    assert(c.iword(index)==91 && c.pword(index)==&marker);
    b=std::move(c);
    assert(b.rdbuf()==bb && b.str()[2]==C('X'));
    b=std::move(b); assert(b.str()[2]==C('X'));
    std::swap(b,c);
    assert(c.str()[2]==C('X'));
    std::basic_stringbuf<C> buffer(text);
    buffer.pubseekpos(3,std::ios_base::in);
    buffer.pubseekpos(2,std::ios_base::out);
    std::basic_stringbuf<C> moved(std::move(buffer));
    assert(moved.sgetc()==C('4'));
    moved.sputc(C('y')); assert(moved.str()[2]==C('y'));
    buffer.str(String(1,C('q')));
    assert(moved.str()[2]==C('y'));
    std::basic_istringstream<C> in(text);
    in.get();
    std::basic_istringstream<C> in2(std::move(in));
    assert(in2.gcount()==1 && in2.get()==C('2'));
    std::basic_ostringstream<C> out;
    out.put(C('a'));
    std::basic_ostringstream<C> out2(std::move(out));
    out2.put(C('b')); assert(out2.str().size()==2);
    out=std::move(out2); assert(out.str().size()==2);
}
int main() {
    static_assert(!std::is_copy_constructible<std::stringstream>::value,"noncopyable");
    static_assert(std::is_move_constructible<std::stringstream>::value,"movable");
    check<char>(); check<wchar_t>();
    assert(events==2);
}
