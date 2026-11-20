// Original regex foundation: flags, locale facets, classes and diagnostics.
// UNSUPPORTED: c++98, c++03
#include <regex>
#include <cassert>
#include <cstring>
struct Fold : std::ctype<char> {
    Fold() : std::ctype<char>() {}
    char do_tolower(char c) const { return c=='@'?'a':std::ctype<char>::do_tolower(c); }
};
template<class C> void traits() {
    std::regex_traits<C> t;
    const C word[]={C('w')},blank[]={C('b'),C('l'),C('a'),C('n'),C('k')};
    auto w=t.lookup_classname(word,word+1);
    assert(t.isctype(C('_'),w) && t.isctype(C('A'),w) && !t.isctype(C('-'),w));
    auto b=t.lookup_classname(blank,blank+5);
    assert(t.isctype(C(' '),b) && t.isctype(C('\t'),b) && !t.isctype(C('\n'),b));
    assert(!t.isctype(C('A'),0));
    assert(t.value(C('f'),16)==15 && t.value(C('f'),10)==-1 && t.value(C('8'),8)==-1);
    assert(t.translate_nocase(C('A'))==C('a'));
    const C nul[]={C('N'),C('U'),C('L')};
    auto name=t.lookup_collatename(nul,nul+3); assert(name.size()==1 && name[0]==C(0));
}
int main() {
    traits<char>(); traits<wchar_t>();
    std::regex_traits<char> t;
    std::locale custom(std::locale::classic(),new Fold);
    std::locale old=t.imbue(custom); assert(old==std::locale());
    assert(t.translate_nocase('@')=='a' && t.translate('@')=='@' && t.getloc()==custom);
    assert(t.imbue(old)==custom);
    using namespace std::regex_constants;
    syntax_option_type flags=icase|nosubs; flags^=nosubs; assert(flags==icase);
    match_flag_type match=match_not_bol|format_no_copy; match&=~match_not_bol; assert(match==format_no_copy);
    try { throw std::regex_error(error_range); }
    catch(const std::regex_error& e) { assert(e.code()==error_range && std::strlen(e.what())>0); std::regex_error copy=e; assert(copy.code()==e.code()); }
}
