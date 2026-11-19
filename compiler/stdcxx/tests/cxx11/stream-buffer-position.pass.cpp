// Original regression: input mode does not imply output-at-end positioning.
#include <sstream>
#include <cassert>
template<class C> void check() {
    const C text[] = {C('a'),C('b'),C('c'),0};
    std::basic_stringstream<C> stream(text);
    stream.put(C('x'));
    assert(stream.str().size()==3 && stream.str()[0]==C('x'));
    assert(stream.get()==C('x'));
    std::basic_stringbuf<C> buf(text,std::ios::in|std::ios::out);
    assert(buf.pubseekoff(0,std::ios::cur,std::ios::out)==0);
    assert(buf.sputc(C('y'))==C('y'));
    assert(buf.str()[0]==C('y') && buf.str().size()==3);
    std::basic_stringbuf<C> end(text,std::ios::in|std::ios::out|std::ios::ate);
    assert(end.pubseekoff(0,std::ios::cur,std::ios::out)==3);
    end.sputc(C('d')); assert(end.str().size()==4);
}
int main() { check<char>(); check<wchar_t>(); }
