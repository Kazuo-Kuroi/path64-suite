// Original checks for protected base operations without optional locales.
// UNSUPPORTED: c++98, c++03
#include <sstream>
#include <utility>
#include <cassert>
struct Input : std::istream {
    explicit Input(std::streambuf* b) : std::istream(b) {}
    Input(Input&& x) : std::istream(std::move(x)) {}
    Input& operator=(Input&& x) { std::istream::operator=(std::move(x)); return *this; }
    using std::istream::swap;
};
struct Output : std::ostream {
    explicit Output(std::streambuf* b) : std::ostream(b) {}
    Output(Output&& x) : std::ostream(std::move(x)) {}
    using std::ostream::swap;
};
struct IO : std::iostream {
    explicit IO(std::streambuf* b) : std::iostream(b) {}
    IO(IO&& x) : std::iostream(std::move(x)) {}
    using std::iostream::swap;
};
struct Buffer : std::streambuf {
    Buffer() {}
    Buffer(const Buffer& x) : std::streambuf(x) {}
    Buffer& operator=(const Buffer& x) { std::streambuf::operator=(x); return *this; }
    using std::streambuf::swap;
    using std::streambuf::setg;
    using std::streambuf::setp;
};
int main() {
    std::stringbuf b("abc"), other("xyz");
    Input a(&b); a.get();
    a.fill('#'); a.precision(9); a.width(3);
    a.setf(std::ios::hex,std::ios::basefield);
    const auto flags=a.flags();
    a.exceptions(std::ios::eofbit);
    int idx=std::ios::xalloc(); a.iword(idx)=17;
    Output tied(&other); a.tie(&tied);
    Input moved(std::move(a));
    assert(moved.rdbuf()==0 && a.rdbuf()==&b);
    assert(moved.good() && moved.exceptions()==std::ios::eofbit);
    assert(moved.flags()==flags && moved.fill()=='#' && moved.precision()==9);
    assert(moved.width()==3 && moved.iword(idx)==17);
    assert(moved.tie()==&tied && a.tie()==0);
    assert(moved.gcount()==1 && a.gcount()==0);
    Input target(&other); target.swap(moved);
    assert(target.rdbuf()==&other && moved.rdbuf()==0);
    assert(target.gcount()==1 && target.iword(idx)==17);
    Output out(&b); out.fill('@'); out.tie(&tied);
    Output out2(std::move(out));
    assert(out2.rdbuf()==0 && out.rdbuf()==&b && out.tie()==0);
    assert(out2.fill()=='@' && out2.tie()==&tied);
    IO io(&b); io.get(); io.precision(8); io.tie(&tied);
    IO io2(std::move(io));
    assert(io2.rdbuf()==0 && io.rdbuf()==&b);
    assert(io2.gcount()==1 && io.gcount()==0 && io2.precision()==8);
    assert(io2.tie()==&tied && io.tie()==0);
    char left[]="abc", right[]="xyz";
    Buffer buf; buf.setg(left,left+1,left+3); buf.setp(left,left+3);
    Buffer copy(buf); assert(copy.sgetc()=='b');
    Buffer assigned; assigned=buf; assert(assigned.sgetc()=='b');
    Buffer alt; alt.setg(right,right+2,right+3);
    buf.swap(alt);
    assert(buf.sgetc()=='z' && alt.sgetc()=='b');
}
