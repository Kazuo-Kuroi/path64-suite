// Original categories, ADL, messages, errno and cross-translation-unit checks.
// UNSUPPORTED: c++98, c++03
#include <system_error>
#include <ios>
#include <sstream>
#include <cassert>
#include <cstring>
#include <pthread.h>
namespace app {
enum class Code { bad=7 };
enum class Condition { special=6 };
struct Category : std::error_category {
    const char* name() const noexcept { return "app"; }
    std::string message(int n) const { return n==7?"bad app value":"other app value"; }
    std::error_condition default_error_condition(int n) const noexcept {
        return n==7?std::make_error_condition(std::errc::invalid_argument):std::error_condition(n,*this);
    }
    bool equivalent(const std::error_code& code,int value) const noexcept {
        return value==6 && code.value()==17;
    }
};
inline const std::error_category& category() { static Category c; return c; }
inline std::error_code make_error_code(Code n) noexcept { return std::error_code(int(n),category()); }
inline std::error_condition make_error_condition(Condition n) noexcept { return std::error_condition(int(n),category()); }
}
namespace std {
template<> struct is_error_code_enum<app::Code> : true_type {};
template<> struct is_error_condition_enum<app::Condition> : true_type {};
}
extern "C" std::error_code error_code_provider();
extern "C" void error_throw_provider();
extern "C" const std::error_category* error_generic_provider();
extern "C" const std::error_category* error_system_provider();
extern "C" const std::error_category* error_stream_provider();
#ifndef ERROR_CONSUMER_ONLY
extern "C" std::error_code error_code_provider() { return std::error_code(EINVAL,std::system_category()); }
extern "C" void error_throw_provider() { throw std::system_error(error_code_provider(),"provider"); }
extern "C" const std::error_category* error_generic_provider() { return &std::generic_category(); }
extern "C" const std::error_category* error_system_provider() { return &std::system_category(); }
extern "C" const std::error_category* error_stream_provider() { return &std::iostream_category(); }
#endif
#ifndef ERROR_PROVIDER_ONLY
static void* worker(void*) {
    for (unsigned i=0;i<100;++i) {
        errno=E2BIG;
        std::error_code code(EINVAL,std::system_category());
        assert(!code.message().empty() && errno==E2BIG);
        assert(code==std::errc::invalid_argument);
        assert(error_generic_provider()==&std::generic_category());
    }
    return 0;
}
int main() {
    pthread_t threads[4];
    for(unsigned i=0;i<4;++i) assert(pthread_create(threads+i,0,worker,0)==0);
    for(unsigned i=0;i<4;++i) assert(pthread_join(threads[i],0)==0);
    assert(error_generic_provider()==&std::generic_category());
    assert(error_system_provider()==&std::system_category());
    assert(error_stream_provider()==&std::iostream_category());
    assert(error_code_provider()==std::errc::invalid_argument);
    try { error_throw_provider(); assert(false); }
    catch(const std::system_error& e) {
        assert(e.code()==std::errc::invalid_argument);
        assert(e.code().category()==std::system_category());
        assert(std::string(e.what()).find("provider")!=std::string::npos);
    }
    std::error_code code=app::Code::bad;
    assert(code.value()==7 && code.category()==app::category());
    assert(code==std::errc::invalid_argument && std::errc::invalid_argument==code);
    std::error_condition condition=app::Condition::special;
    std::error_code special(17,std::system_category());
    assert(special==condition && condition==special);
    std::error_code unknown(1234567,std::system_category());
    assert(unknown.default_error_condition().category()==std::system_category());
    assert(std::error_code().default_error_condition()==std::error_condition());
    std::system_error error(code,"operation");
    assert(error.code()==code && std::string(error.what()).find("operation")!=std::string::npos);
    assert(std::string(error.what()).find(code.message())!=std::string::npos);
    std::system_error copy(error); assert(copy.code()==code && std::strcmp(copy.what(),error.what())==0);
    std::system_error assigned(std::make_error_code(std::errc::io_error)); assigned=error;
    assert(assigned.code()==code && std::strcmp(assigned.what(),error.what())==0);
    try { throw error; } catch(const std::runtime_error& e) { assert(std::strcmp(e.what(),error.what())==0); }
    std::error_code stream=std::io_errc::stream;
    assert(stream.category()==std::iostream_category() && stream.value()==1);
    std::ostringstream output; output<<stream;
    assert(output.str()=="iostream:1");
    assert(std::hash<std::error_code>()(stream)==std::hash<std::error_code>()(std::make_error_code(std::io_errc::stream)));
    code.clear(); condition.clear();
    assert(!code && !condition && code.category()==std::system_category() && condition.category()==std::generic_category());
}
#endif
