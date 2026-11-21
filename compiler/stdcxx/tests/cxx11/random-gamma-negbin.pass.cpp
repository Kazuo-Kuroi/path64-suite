// Original gamma/negative-binomial CDF, PMF, moments and state regressions.
// UNSUPPORTED: c++98, c++03
#include <random>
#include <algorithm>
#include <sstream>
#include <vector>
#include <cmath>
#include <cassert>
#include <limits>

template<class D> void moments(D d,double mean,double variance,double excess) {
    const unsigned seeds[]={19,313,2027};
    const int n=30000; double sum=0,square=0;
    for (unsigned seed:seeds) {
        std::mt19937 engine(seed);
        for (int i=0;i<n;++i) {
            auto x=d(engine); assert(x>=d.min() && x<=d.max());
            double delta=double(x)-mean; sum+=delta; square+=delta*delta;
        }
    }
    assert(std::abs(sum/(3*n))<6*std::sqrt(variance/(3*n)));
    assert(std::abs(square/(3*n)-variance)<6*variance*std::sqrt((excess+2)/(3*n)));
}
void gamma_cdf(double shape) {
    std::gamma_distribution<> d(shape,2); std::mt19937 engine(451);
    const int n=90000; unsigned counts[13]={}; double expected[13];
    double previous=0;
    for (int i=0;i<12;++i) {
        double x=(i+1)*.5;
        double cdf=shape==.5?std::erf(std::sqrt(x)):1-std::exp(-x)*(shape==1?1:1+x);
        expected[i]=n*(cdf-previous); previous=cdf;
    }
    expected[12]=n*(1-previous);
    for(int i=0;i<n;++i) { double value=d(engine); assert(value>=0); int bucket=value>=12?12:int(value); ++counts[bucket]; }
    double chi=0;
    for(int i=0;i<13;++i) { double diff=counts[i]-expected[i]; chi+=diff*diff/expected[i]; }
    assert(chi<70);
}
void negative_pmf(int k,double p) {
    std::negative_binomial_distribution<> d(k,p); std::mt19937 engine(891);
    const int n=90000; unsigned counts[201]={}; double expected[201];
    double probability=std::pow(p,k),sum=0;
    for(int i=0;i<200;++i) { expected[i]=n*probability; sum+=probability; probability*=double(i+k)/(i+1)*(1-p); }
    expected[200]=n*(std::max)(0.,1-sum);
    for(int i=0;i<n;++i) { int x=d(engine); assert(x>=0); ++counts[x>=200?200:x]; }
    double e=0,o=0,chi=0; int cells=0;
    for(int i=0;i<201;++i) {
        e+=expected[i]; o+=counts[i];
        if(e>=10) { double diff=o-e; chi+=diff*diff/e; e=o=0; ++cells; }
    }
    // The last tiny tail remains an independent bound rather than division by zero.
    assert(o<30 && chi<cells+8*std::sqrt(double(2*cells))+24);
}
template<class D> void state(D d) {
    std::stringstream stream; stream.setf(std::ios::hex,std::ios::basefield); stream.precision(2); stream.fill('*');
    auto flags=stream.flags(); stream<<d;
    assert(stream.flags()==flags && stream.precision()==2 && stream.fill()=='*');
    D copy; stream>>copy; assert(copy==d && stream.flags()==flags);
    std::mt19937 a(21),b(21); for(int i=0;i<100;++i) assert(d(a)==copy(b));
    d.reset(); assert(d==copy);
    std::istringstream truncated("7"); auto saved=d; truncated>>d; assert(truncated.fail() && d==saved);
    std::istringstream throwing("7"); throwing.exceptions(std::ios::failbit);
    try { throwing>>d; assert(false); } catch(const std::ios_base::failure&) {} assert(d==saved);
}
struct NoDraw {
    typedef unsigned result_type;
    static constexpr unsigned min() { return 0; }
    static constexpr unsigned max() { return 0xffffffffU; }
    unsigned operator()() { assert(false); return 0; }
};
int main() {
    for(double shape:{.125,.5,1.,2.,40.}) moments(std::gamma_distribution<>(shape,2),shape*2,shape*4,6/shape);
    gamma_cdf(.5); gamma_cdf(1); gamma_cdf(2);
    for(double p:{.25,.8}) { double k=5,mean=k*(1-p)/p; moments(std::negative_binomial_distribution<>(5,p),mean,mean/p,6/k+p*p/(k*(1-p))); }
    negative_pmf(5,.25); negative_pmf(1,.6); negative_pmf(100,.8);
    moments(std::gamma_distribution<float>(.75f,2),1.5,3,8);
    moments(std::gamma_distribution<long double>(2,3),6,18,3);
    moments(std::negative_binomial_distribution<long long>(1000000000LL,.4),1500000000.,3750000000.,0);
    state(std::gamma_distribution<float>(.125f,1.234567f));
    state(std::gamma_distribution<double>(.125,1.2345678901234567));
    state(std::gamma_distribution<long double>(.125L,1.234567890123456789L));
    state(std::negative_binomial_distribution<>(5,.12345678901234567));
    NoDraw no; assert(std::negative_binomial_distribution<>(123,1)(no)==0);
    std::mt19937 engine(71); std::gamma_distribution<> gamma(3,4);
    auto g=gamma(engine,decltype(gamma)::param_type(.5,2)); assert(g>=0 && gamma.alpha()==3 && gamma.beta()==4);
    std::negative_binomial_distribution<> neg(5,.25);
    auto x=neg(engine,decltype(neg)::param_type(2,.8)); assert(x>=0 && neg.k()==5 && neg.p()==.25);
}
