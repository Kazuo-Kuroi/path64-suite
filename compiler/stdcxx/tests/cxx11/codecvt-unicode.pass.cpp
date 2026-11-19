// Original Unicode boundaries, truncation, BOM and file-facet integration checks.
// UNSUPPORTED: c++98, c++03
#include <codecvt>
#include <fstream>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
int main() {
    std::codecvt_utf8<char32_t> utf8;
    std::codecvt_utf8<char32_t>::result status=std::codecvt_base::ok;
    assert(status==std::codecvt_base::ok);
    const char32_t values[]={0,1,0x7f,0x80,0x7ff,0x800,0xd7ff,0xe000,0xffff,0x10000,0x10ffff};
    for (unsigned i=0;i<sizeof(values)/sizeof(*values);++i) {
        char bytes[8]={}; char* out=bytes;
        const char32_t* next=values+i;
        std::mbstate_t state={};
        assert(utf8.out(state,values+i,values+i+1,next,bytes,bytes+8,out)==std::codecvt_base::ok);
        assert(next==values+i+1);
        const std::size_t n=out-bytes;
        char32_t cp=0; char32_t* end=&cp; const char* consumed=bytes;
        assert(utf8.in(state,bytes,out,consumed,&cp,&cp+1,end)==std::codecvt_base::ok);
        assert(cp==values[i] && consumed==out && end==&cp+1);
        assert(utf8.length(state,bytes,out,1)==int(n));
        for (std::size_t k=1;k<n;++k) {
            consumed=bytes; end=&cp;
            assert(utf8.in(state,bytes,bytes+k,consumed,&cp,&cp+1,end)==std::codecvt_base::partial);
            assert(consumed==bytes && end==&cp);
        }
        for (std::size_t k=0;k<n;++k) {
            char shortbuf[8]; for(unsigned j=0;j<8;++j) shortbuf[j]='!';
            char* cursor=shortbuf; next=values+i;
            assert(utf8.out(state,values+i,values+i+1,next,shortbuf,shortbuf+k,cursor)==std::codecvt_base::partial);
            assert(next==values+i && cursor==shortbuf && shortbuf[k]=='!');
        }
    }
    const unsigned char invalid[][4]={{0xc0,0x80,0,0},{0xed,0xa0,0x80,0},
        {0xf4,0x90,0x80,0x80},{0xf5,0x80,0x80,0x80},{0xe2,0x28,0xa1,0}};
    const unsigned lengths[]={2,3,4,4,3};
    for(unsigned i=0;i<5;++i) {
        const char* b=reinterpret_cast<const char*>(invalid[i]); const char* next=b;
        char32_t cp=0; char32_t* out=&cp; std::mbstate_t state={};
        assert(utf8.in(state,b,b+lengths[i],next,&cp,&cp+1,out)==std::codecvt_base::error);
        assert(next==b && out==&cp);
    }
    std::codecvt_utf8_utf16<char32_t> pairs;
    const char smile[]={char(0xf0),char(0x9f),char(0x98),char(0x80)};
    char32_t units[2]={}; char32_t* out=units; const char* next=smile;
    std::mbstate_t state={};
    assert(pairs.in(state,smile,smile+4,next,units,units+1,out)==std::codecvt_base::partial);
    assert(next==smile && out==units);
    assert(pairs.in(state,smile,smile+4,next,units,units+2,out)==std::codecvt_base::ok);
    assert(units[0]==0xd83d && units[1]==0xde00);
    const char32_t bad=0x10000; const char32_t* unitnext=&bad;
    char encoded[8]; char* cursor=encoded;
    assert(pairs.out(state,&bad,&bad+1,unitnext,encoded,encoded+8,cursor)==std::codecvt_base::error);
    typedef std::codecvt_utf16<char32_t,0x10ffff,
        std::codecvt_mode(std::little_endian|std::generate_header|std::consume_header)> LE;
    LE little;
    const char32_t face=0x1f600; unitnext=&face; cursor=encoded;
    assert(little.out(state,&face,&face+1,unitnext,encoded,encoded+8,cursor)==std::codecvt_base::ok);
    assert(cursor==encoded+6 && static_cast<unsigned char>(encoded[0])==0xff &&
           static_cast<unsigned char>(encoded[1])==0xfe);
    char32_t result=0; out=&result; next=encoded;
    assert(little.in(state,encoded,cursor,next,&result,&result+1,out)==std::codecvt_base::ok && result==face);
    assert(little.length(state,encoded,cursor,1)==6);
    char path[]="/tmp/path64-codecvt-XXXXXX";
    int fd=mkstemp(path); assert(fd>=0); close(fd);
    std::locale loc(std::locale::classic(),new std::codecvt_utf8<wchar_t>);
    std::wfstream file;
    file.imbue(loc);
    file.open(std::string(path),std::ios::in|std::ios::out|std::ios::trunc);
    assert(file.is_open());
    file << L"A\u20ac\U0001f600"; file.flush(); file.seekg(0);
    assert(file.get()==L'A' && file.get()==0x20ac && file.get()==0x1f600);
    file.close();
    std::filebuf raw;
    assert(raw.open(std::string(path),std::ios::in)); assert(raw.close());
    std::ifstream input{std::string(path)}; assert(input.is_open()); input.close();
    std::ofstream append(std::string(path),std::ios::app); assert(append.is_open()); append.close();
    assert(std::remove(path)==0);
}
