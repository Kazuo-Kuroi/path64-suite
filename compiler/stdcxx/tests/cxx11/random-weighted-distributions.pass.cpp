// Original weighted-distribution normalization, CDF and stream regressions.
// UNSUPPORTED: c++98, c++03
#include <random>
#include <sstream>
#include <iterator>
#include <vector>
#include <limits>
#include <cmath>
#include <cassert>

template<class D,class F> void cdf_bins(D d,F cdf) {
    const unsigned seeds[]={23,421,2053}; unsigned bins[10]={}; const int n=30000;
    for(unsigned seed:seeds) {
        std::mt19937 engine(seed);
        for(int i=0;i<n;++i) {
            auto x=d(engine); assert(x>=d.min() && x<d.max());
            double p=cdf(double(x)); assert(p>=0 && p<=1);
            ++bins[p==1?9:int(p*10)];
        }
    }
    double chi=0; for(unsigned count:bins) { double delta=double(count)-3*n/10; chi+=delta*delta/(3*n/10); }
    assert(chi<75);
}
template<class D> void state(D d,typename D::param_type alternate) {
    auto saved=d; D other(alternate); std::mt19937 a(29),b(29);
    for(int i=0;i<40;++i) assert(d(a,alternate)==other(b)); assert(d==saved);
    d.reset(); assert(d==saved);
    std::stringstream io; io.setf(std::ios::hex,std::ios::basefield); io.precision(2); io.fill('*'); io.width(8);
    auto flags=io.flags(); io<<d;
    assert(io.flags()==flags && io.precision()==2 && io.fill()=='*');
    D copy; io>>copy; assert(copy==d && io.flags()==flags);
    std::mt19937 c(31),e(31); for(int i=0;i<40;++i) assert(copy(c)==d(e));
    std::wstringstream wide; wide<<d; D wide_copy; wide>>wide_copy; assert(wide_copy==d);
    std::istringstream truncated("3 0"); truncated>>d; assert(truncated.fail() && d==saved);
    std::istringstream throwing("3 0"); throwing.exceptions(std::ios::failbit);
    try { throwing>>d; assert(false); } catch(const std::ios_base::failure&) {} assert(d==saved);
}
template<class D> void reject(D d,const char* text) {
    auto saved=d; std::istringstream input(text); input>>d; assert(input.fail() && d==saved);
    std::istringstream throwing(text); throwing.exceptions(std::ios::failbit);
    try { throwing>>d; assert(false); } catch(const std::ios_base::failure&) {} assert(d==saved);
}
struct ZeroEngine {
    typedef unsigned result_type;
    static constexpr unsigned min() { return 0; }
    static constexpr unsigned max() { return 0xffffffffU; }
    unsigned operator()() { return 0; }
};
struct MaxEngine : ZeroEngine { unsigned operator()() { return max(); } };
template<class R> void types() {
    const R b[]={R(-2.123456789012345678L),0,R(.125),R(3.4567890123456789L)};
    const double constant[]={0,2,7},linear[]={0,2,1,0};
    typedef std::piecewise_constant_distribution<R> C;
    typedef std::piecewise_linear_distribution<R> L;
    state(C(b,b+4,constant),typename C::param_type());
    state(L(b,b+4,linear),typename L::param_type());
    R tiny[]={R(1),std::nextafter(R(1),R(2))}; double flat[]={1,1}; MaxEngine maximum;
    C c(tiny,tiny+2,flat); L l(tiny,tiny+2,flat);
    assert(c(maximum)==tiny[0] && l(maximum)==tiny[0]);
}
int main() {
    std::discrete_distribution<> d({0,2,0,8,0});
    assert(d.probabilities()[1]==.2 && d.probabilities()[3]==.8 && d.min()==0 && d.max()==4);
    for(unsigned seed:{23,421,2053}) {
        std::mt19937 engine(seed); int count=0;
        for(int i=0;i<90000;++i) { int x=d(engine); assert(x==1 || x==3); count+=x==1; }
        assert(std::abs(count-18000.)<6*std::sqrt(90000*.2*.8));
    }
    ZeroEngine zero; assert(d(zero)==1);
    std::discrete_distribution<> empty({}); assert(empty(zero)==0 && empty.probabilities()==std::vector<double>(1,1));
    state(d,decltype(d)::param_type({1,0,3}));
    state(std::discrete_distribution<>({1e-310,1}),decltype(d)::param_type());
    state(std::discrete_distribution<unsigned long long>({1,7,2}),std::discrete_distribution<unsigned long long>::param_type({2,1}));
    reject(d,"0"); reject(d,"3 0 0 0"); reject(d,"2 1 -1"); reject(d,"-1");
    double huge=(std::numeric_limits<double>::max)();
    std::discrete_distribution<> large({huge,huge,0}); assert(large.probabilities()[0]==.5 && large.probabilities()[1]==.5);
    std::istringstream ws("1 3 0"); std::discrete_distribution<> input((std::istream_iterator<double>(ws)),std::istream_iterator<double>());
    assert(input.probabilities()[0]==.25 && input.probabilities()[1]==.75);
    std::vector<double> calls;
    std::discrete_distribution<> functional(2,0,4,[&](double x){ calls.push_back(x); return x; });
    assert(calls.size()==2 && calls[0]==1 && calls[1]==3 && functional.probabilities()[0]==.25);
    const double b[]={0,1,3,4},cw[]={1,0,3},lw[]={0,2,0,0};
    std::piecewise_constant_distribution<> constant(b,b+4,cw);
    assert(constant.densities()[0]==.25 && constant.densities()[1]==0 && constant.densities()[2]==.75);
    cdf_bins(constant,[](double x){ assert(x<1 || x>=3); return x<1?.25*x:.25+.75*(x-3); });
    std::piecewise_linear_distribution<> linear(b,b+4,lw);
    cdf_bins(linear,[](double x){ assert(x<3); double t=x-1; return x<1?x*x/3:(1+2*t-.5*t*t)/3; });
    const double unit[]={0,1},rising[]={0,2},falling[]={2,0},nearflat[]={1,1+1e-12};
    cdf_bins(std::piecewise_linear_distribution<>(unit,unit+2,rising),[](double x){return x*x;});
    cdf_bins(std::piecewise_linear_distribution<>(unit,unit+2,falling),[](double x){return x*(2-x);});
    cdf_bins(std::piecewise_linear_distribution<>(unit,unit+2,nearflat),[](double x){return (x+.5e-12*x*x)/(1+.5e-12);});
    const double gap[]={0,1,2},skip[]={0,1},skiplinear[]={0,0,1};
    assert(std::piecewise_constant_distribution<>(gap,gap+3,skip)(zero)==1);
    assert(std::piecewise_linear_distribution<>(gap,gap+3,skiplinear)(zero)==1);
    types<float>(); types<double>(); types<long double>();
    // Weights are normalized before narrowing to the result type.
    double weights[]={huge,huge}; float bounds[]={0,1,2};
    std::piecewise_constant_distribution<float> hc(bounds,bounds+3,weights);
    std::piecewise_linear_distribution<float> hl(bounds,bounds+2,weights);
    assert(hc.densities()[0]==.5f && hl.densities()[0]==1 && hl.densities()[1]==1);
    cdf_bins(hc,[](double x){return x/2;});
    double widebounds[]={-1e308,1e308};
    std::piecewise_linear_distribution<> wide(widebounds,widebounds+2,weights);
    state(wide,decltype(wide)::param_type());
    std::mt19937 engine(41); for(int i=0;i<100;++i) { double x=wide(engine); assert(std::isfinite(x) && x>=-1e308 && x<1e308); }
    reject(constant,"2 1 0 1 1"); reject(constant,"3 0 1 2 1 1"); reject(constant,"2 0 1 1 0");
    reject(linear,"2 0 1 2 1 -1"); reject(linear,"2 0 1 1 1");
    calls.clear(); std::piecewise_constant_distribution<> functions({0,1,3},[&](double x){calls.push_back(x);return 1;});
    assert(calls.size()==2 && calls[0]==.5 && calls[1]==2);
    calls.clear(); std::piecewise_linear_distribution<> endpoints(0,2.,4.,[&](double x){calls.push_back(x);return 1;});
    assert(calls.size()==2 && calls[0]==2 && calls[1]==4 && endpoints.densities()[0]==.5);
}
