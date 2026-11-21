// Original ECMAScript search, captures, formatting, flags and resource checks.
// UNSUPPORTED: c++98, c++03
#include <regex>
#include <list>
#include <cassert>
#include <string>
#include <utility>
int main() {
    std::cmatch m;
    assert(!m.ready() && m.empty());
    assert(std::regex_search("abcdefghijk",m,std::regex("cd((e)fg)hi")));
    assert(m.ready() && m.size()==3 && m.str()=="cdefghi" && m.str(1)=="efg" && m.str(2)=="e");
    assert(m.position()==2 && m.position(2)==4 && m.length()==7 && m.prefix()=="ab" && m.suffix()=="jk");
    assert(m.format("$`|$&|$1|$2|$'|$$|$99")=="ab|cdefghi|efg|e|jk|$|");
    assert(m.format("<&>\\1",std::regex_constants::format_sed)=="<cdefghi>efg");
    auto saved=m; assert(saved==m);
    assert(!std::regex_search("xyz",m,std::regex("q")) && m.ready() && m.empty());
    assert(m.begin()==m.end() && !m[3].matched && m.str().empty());
    assert(saved!=m); saved.swap(m); assert(m.size()==3 && saved.empty());
    assert(std::regex_search("xx AA123! yy",m,std::regex("([a-z]+)(\\d+)",std::regex::icase)));
    assert(m.str()=="AA123" && m.str(1)=="AA" && m.str(2)=="123");
    assert(std::regex_search("aaaa",m,std::regex("a+?")) && m.str()=="a");
    assert(std::regex_search("aaaa",m,std::regex("a+")) && m.str()=="aaaa");
    assert(std::regex_match("abab",m,std::regex("(ab)\\1")));
    assert(std::regex_match("b",m,std::regex("(a)?b\\1")) && !m[1].matched && m.position(1)==1);
    assert(std::regex_match("ab",m,std::regex("(?:(a)|b)+")) && !m[1].matched);
    assert(std::regex_match("",m,std::regex("(a?)*")) && m[1].matched && m[1].length()==0);
    assert(std::regex_match("aaa",std::regex("a{2,4}")) && !std::regex_match("a",std::regex("a{2,4}")));
    assert(std::regex_match("a",std::regex("a|ab")) && std::regex_match("ab",std::regex("a|ab")));
    assert(std::regex_match("x7_",std::regex("[a-z][[:digit:]][\\w]")));
    assert(!std::regex_match("\n",std::regex(".")) && std::regex_match("\n",std::regex("[^]")));
    assert(std::regex_search(" x! ",std::regex("\\bx\\b")) && !std::regex_search("ax",std::regex("\\bx")));
    assert(!std::regex_search("x",std::regex("^x"),std::regex_constants::match_not_bol));
    assert(!std::regex_search("x",std::regex("x$"),std::regex_constants::match_not_eol));
    assert(!std::regex_search("abc",std::regex(""),std::regex_constants::match_not_null));
    assert(!std::regex_search("ba",std::regex("a"),std::regex_constants::match_continuous));
    assert(std::regex_search("a",m,std::regex("(a)",std::regex::nosubs)) && m.size()==1);
    std::regex re("ok");
    try { re.assign("[z-a]"); assert(false); } catch(const std::regex_error& e) { assert(e.code()==std::regex_constants::error_range); }
    assert(std::regex_match("ok",re));
    std::regex moved(std::move(re)); assert(std::regex_match("ok",moved)); assert(!std::regex_match("ok",re)); re.assign("new"); assert(std::regex_match("new",re));
    const char embedded[]={'a',0,'b'}; assert(std::regex_match(embedded,embedded+3,std::regex("a.b")));
    std::list<char> chars; chars.push_back('a'); chars.push_back('b');
    std::match_results<std::list<char>::const_iterator> list_match;
    assert(std::regex_search(chars.cbegin(),chars.cend(),list_match,std::regex("(ab)")) && list_match.str(1)=="ab");
    std::wcmatch wm; assert(std::regex_search(L"123 xyz",wm,std::wregex(L"(\\d+)\\s+(\\w+)")) && wm.str(2)==L"xyz");
    const char high_byte[]={char(0xff)}; assert(std::regex_match(high_byte,high_byte+1,std::regex("[\\x00-\\xff]+")));
    assert(std::regex_match("AB",std::regex("\\x41\\u0042")));
    assert(std::regex_match(std::string(10000,'a'),std::regex("a*")));
    try { std::regex bad_escape("\\a"); assert(false); } catch(const std::regex_error& e) { assert(e.code()==std::regex_constants::error_escape); }
    try { std::regex invalid("\\2(a)"); assert(false); } catch(const std::regex_error& e) { assert(e.code()==std::regex_constants::error_backref); }
    try { std::regex unsupported("a",std::regex::extended); assert(false); } catch(const std::regex_error&) {}
    try { std::regex huge(std::string(9000,'a')); assert(false); } catch(const std::regex_error& e) { assert(e.code()==std::regex_constants::error_space); }
    try { std::regex_match(std::string(40,'a')+"!",std::regex("(a|aa)*")); assert(false); }
    catch(const std::regex_error& e) { assert(e.code()==std::regex_constants::error_complexity || e.code()==std::regex_constants::error_stack); }
}
