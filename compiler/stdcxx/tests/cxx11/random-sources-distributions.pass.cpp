// Original entropy-source errors and stateful distribution regression.
// UNSUPPORTED: c++98, c++03
#include <random>
#include <sstream>
#include <cassert>
#include <cmath>
#include <cerrno>
#include <cstring>
#include <type_traits>
#ifdef __linux__
#include <sys/syscall.h>
static volatile int read_mode,read_calls;
extern "C" ssize_t read(int fd,void* data,size_t size) {
    if (!read_mode) return ::syscall(SYS_read,fd,data,size);
    ++read_calls;
    if (read_mode==2) return 0;
    if (read_mode==3) { errno=EACCES; return -1; }
    if (read_calls==1) { errno=EINTR; return -1; }
    size_t count=read_calls==2?1:size;
    std::memset(data,read_calls==2?0xab:0xcd,count); return ssize_t(count);
}
#endif
struct Counting {
    typedef std::mt19937::result_type result_type;
    std::mt19937 engine; unsigned calls;
    Counting() : engine(123),calls(0) {}
    static constexpr result_type min() { return std::mt19937::min(); }
    static constexpr result_type max() { return std::mt19937::max(); }
    result_type operator()() { ++calls; return engine(); }
};
template<class D> void roundtrip(D d) {
    Counting g; d(g); D original=d;
    std::stringstream stream; stream.setf(std::ios::hex,std::ios::basefield); stream.precision(3); stream.fill('?'); stream.width(24);
    auto flags=stream.flags(); stream<<d;
    assert(stream.flags()==flags && stream.precision()==3 && stream.fill()=='?');
    D copy; stream>>copy; assert(copy==original && stream.flags()==flags);
    Counting other=g;
    for (int i=0;i<20;++i) assert(copy(other)==d(g));
}
int main() {
    static_assert(!std::is_copy_constructible<std::random_device>::value,"entropy source owns its descriptor");
    try { std::random_device invalid("/dev/null"); assert(false); }
    catch(const std::system_error& e) { assert(e.code().value()==EINVAL); }
    {
        std::random_device device;
        assert(device.entropy()>=0 && device.entropy()<=std::numeric_limits<unsigned>::digits);
        (void)device();
#ifdef __linux__
        read_mode=1; read_calls=0; unsigned value=device();
        unsigned char expected[sizeof(value)]; std::memset(expected,0xcd,sizeof(expected)); expected[0]=0xab;
        assert(std::memcmp(&value,expected,sizeof(value))==0 && read_calls==3);
        read_mode=2;
        try { device(); assert(false); } catch(const std::system_error& e) { assert(e.code().value()==EIO); }
        read_mode=3;
        try { device(); assert(false); } catch(const std::system_error& e) { assert(e.code().value()==EACCES); }
        read_mode=0; (void)device();
#endif
    }
    int saved=::dup(0); ::close(0);
    { std::random_device zero; assert((::fcntl(0,F_GETFD)&FD_CLOEXEC)!=0); errno=EDOM; }
    assert(errno==EDOM); assert(::fcntl(0,F_GETFD)==-1 && errno==EBADF);
    if (saved>=0) { assert(::dup2(saved,0)==0); ::close(saved); }
    Counting g; std::normal_distribution<double> normal;
    (void)normal(g); unsigned calls=g.calls; auto copy=normal;
    double z=copy(g),transformed=normal(g,decltype(normal)::param_type(100,2));
    assert(g.calls==calls && std::abs(transformed-(100+2*z))<1e-12);
    normal.reset(); (void)normal(g); assert(g.calls>calls);
    roundtrip(std::normal_distribution<double>(2.25,1.75));
    roundtrip(std::lognormal_distribution<double>(0.25,0.5));
    roundtrip(std::exponential_distribution<double>(2.5));
    auto original=normal; std::istringstream truncated("1 2");
    truncated>>normal; assert(truncated.fail() && normal==original);
    std::istringstream throwing("1 2"); throwing.exceptions(std::ios::failbit);
    try { throwing>>normal; assert(false); } catch(const std::ios_base::failure&) {}
    assert(normal==original);
    std::normal_distribution<double> n(2,3); std::exponential_distribution<double> e(2);
    double sum=0,squares=0,exponential=0;
    for (int i=0;i<30000;++i) { double x=n(g); sum+=x; squares+=(x-2)*(x-2); double y=e(g); assert(y>=0); exponential+=y; }
    assert(std::abs(sum/30000-2)<0.1 && std::abs(squares/30000-9)<0.4 && std::abs(exponential/30000-0.5)<0.02);
}
