// Original capture-range semantics, embedded NULs and bidirectional iterators.
// UNSUPPORTED: c++98, c++03
#include <regex>
#include <list>
#include <sstream>
#include <cassert>
#include <type_traits>
constexpr std::csub_match empty;
static_assert(!empty.matched && empty.first==nullptr && empty.second==nullptr,"empty capture");
int main() {
    const char text[]={'a',0,'b'};
    std::csub_match capture;
    capture.first=text; capture.second=text+3;
    assert(capture.length()==0 && capture.str().empty() && capture=="");
    capture.matched=true;
    assert(capture.length()==3 && capture.str()==std::string(text,3));
    assert(capture>"a" && "a"<capture && capture!=std::string("a"));
    auto copy=capture; assert(copy==capture); copy.matched=false; assert(copy<capture);
    std::ostringstream out; out.width(5); out<<capture;
    assert(out.str()==std::string("  ")+std::string(text,3));
    std::list<char> chars; chars.push_back('x'); chars.push_back('y');
    std::sub_match<std::list<char>::const_iterator> linked;
    linked.first=chars.begin(); linked.second=chars.end(); linked.matched=true;
    assert(linked.length()==2 && linked==std::string("xy") && linked>'x');
    const wchar_t wide[]=L"z"; std::wcsub_match w; w.first=wide; w.second=wide+1; w.matched=true;
    assert(w==L'z' && L'z'==w && L'a'<w && w>L'a');
    std::wostringstream stream; stream<<w; assert(stream.str()==L"z");
}
