// Original ownership, buffered input/output and external-buffer move checks.
// UNSUPPORTED: c++98, c++03
#include <fstream>
#include <utility>
#include <cassert>
#include <cstdio>
#include <unistd.h>
template<class C> void check(const char* path, const char* second) {
    typedef std::basic_fstream<C> Stream;
    Stream a(path,std::ios::in|std::ios::out|std::ios::trunc);
    assert(a.is_open());
    a.put(C('a')); a.put(C('b')); a.put(C('c')); a.seekg(0);
    assert(a.get()==C('a'));
    Stream b(std::move(a));
    assert(!a.is_open() && b.is_open());
    assert(b.get()==C('b') && b.get()==C('c'));
    auto* bp=b.rdbuf();
    Stream c(second,std::ios::in|std::ios::out|std::ios::trunc);
    c.put(C('x')); c.seekg(0);
    auto* cp=c.rdbuf();
    b.swap(c);
    assert(b.rdbuf()==bp && c.rdbuf()==cp);
    assert(b.get()==C('x'));
    b=std::move(c);
    assert(b.is_open() && !c.is_open());
    b.clear(); b.seekp(0); b.put(C('z')); b.close();
    a.open(second,std::ios::in|std::ios::out);
    assert(a.is_open() && a.get()==C('x')); a.close();
    std::basic_ifstream<C> input(path);
    assert(input.get()==C('z'));
    std::basic_ifstream<C> next(std::move(input));
    assert(!input.is_open() && next.get()==C('b'));
    std::basic_ofstream<C> output(second,std::ios::app);
    output.put(C('y'));
    std::basic_ofstream<C> more(std::move(output));
    assert(!output.is_open() && more.is_open()); more.put(C('z')); more.close();
    C external[32];
    std::basic_filebuf<C> buf;
    buf.pubsetbuf(external,32);
    assert(buf.open(path,std::ios::in));
    assert(buf.sbumpc()==C('z'));
    std::basic_filebuf<C> transferred(std::move(buf));
    assert(!buf.is_open() && transferred.sbumpc()==C('b'));
    std::basic_filebuf<C> assigned;
    assigned=std::move(transferred);
    assert(!transferred.is_open() && assigned.sbumpc()==C('c'));
    assigned=std::move(assigned); assert(assigned.is_open());
}
int main() {
    char path[]="/tmp/path64-stream-move-XXXXXX";
    char second[]="/tmp/path64-stream-move-XXXXXX";
    int fd=mkstemp(path); assert(fd>=0); close(fd);
    fd=mkstemp(second); assert(fd>=0); close(fd);
    check<char>(path,second); check<wchar_t>(path,second);
    assert(std::remove(path)==0 && std::remove(second)==0);
}
