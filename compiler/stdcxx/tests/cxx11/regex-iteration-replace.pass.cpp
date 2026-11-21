// Original iterator progression, empty matches and replacement regression.
// UNSUPPORTED: c++98, c++03
#include <regex>
#include <list>
#include <cassert>
#include <vector>
struct TextIterator {
    typedef std::bidirectional_iterator_tag iterator_category;
    typedef char value_type; typedef std::ptrdiff_t difference_type;
    typedef const char* pointer; typedef const char& reference;
    pointer p;
    TextIterator(pointer value=0) : p(value) {}
    reference operator*() const { return *p; }
    TextIterator& operator++() { ++p; return *this; }
    TextIterator operator++(int) { auto old=*this; ++*this; return old; }
    TextIterator& operator--() { --p; return *this; }
    TextIterator operator--(int) { auto old=*this; --*this; return old; }
    friend bool operator==(TextIterator a,TextIterator b) { return a.p==b.p; }
    friend bool operator!=(TextIterator a,TextIterator b) { return !(a==b); }
};
template<class U> void operator,(const TextIterator&,const U&)=delete;
int main() {
    std::string text="ab12 cd345!"; std::regex digits("(\\d+)");
    std::sregex_iterator i(text.begin(),text.end(),digits),end; auto copy=i;
    assert(i==copy && i->position()==2 && i->str(1)=="12" && i->prefix()=="ab");
    auto first=i++; assert(first==copy && first->str()=="12");
    assert(i!=copy && i->position()==7 && i->str()=="345" && i->prefix()==" cd");
    ++i; assert(i==end); ++copy; assert(copy->position()==7);
    std::regex empty_or_a("|a"); std::string a="a";
    std::sregex_iterator e(a.begin(),a.end(),empty_or_a);
    assert(e->length()==0 && e->position()==0); ++e;
    assert(e->str()=="a" && e->position()==0); ++e;
    assert(e->length()==0 && e->position()==1); ++e; assert(e==end);
    std::string spaced=" a"; std::regex anchored("\\b|^a");
    std::sregex_iterator anchors(spaced.begin(),spaced.end(),anchored);
    assert(anchors->position()==1 && anchors->length()==0); ++anchors;
    assert(anchors->position()==2 && anchors->length()==0); ++anchors; assert(anchors==end);
    std::regex empty(""); std::string ab="ab";
    std::sregex_iterator boundary(ab.begin(),ab.end(),empty);
    assert(boundary->prefix().length()==0); ++boundary;
    assert(boundary->position()==1 && boundary->prefix()=="a"); ++boundary;
    assert(boundary->position()==2 && boundary->prefix()=="b"); ++boundary; assert(boundary==end);
    assert(std::regex_replace(ab,empty,"X")=="XaXbX");
    assert(std::regex_replace(a,empty_or_a,"X")=="XXX");
    assert(std::regex_replace(text,digits,"<$1>")=="ab<12> cd<345>!");
    assert(std::regex_replace(text,digits,"X",std::regex_constants::format_first_only)=="abX cd345!");
    assert(std::regex_replace(text,digits,"$&",std::regex_constants::format_no_copy)=="12345");
    assert(std::regex_replace(text,digits,"&-\\1",std::regex_constants::format_sed)=="ab12-12 cd345-345!");
    assert(std::regex_replace("abc",std::regex("z"),"X")=="abc");
    assert(std::regex_replace("abc",std::regex("z"),"X",std::regex_constants::format_no_copy).empty());
    std::string format("x\0y",3); auto embedded=std::regex_replace("a",std::regex("a"),format); assert(embedded==format);
    const char raw[]="a1b2"; std::string output;
    std::regex_replace(std::back_inserter(output),TextIterator(raw),TextIterator(raw+4),digits,"<$1>");
    assert(output=="a<1>b<2>");
    std::list<char> linked; linked.push_back('a'); linked.push_back('1'); linked.push_back('2');
    std::regex_iterator<std::list<char>::const_iterator> list_it(linked.cbegin(),linked.cend(),digits);
    assert(list_it->str()=="12" && list_it->position()==1);
    assert(std::regex_replace(L"x12y",std::wregex(L"(\\d+)"),L"<$1>")==L"x<12>y");
}
