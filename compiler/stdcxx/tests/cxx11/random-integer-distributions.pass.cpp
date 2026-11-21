// Original integer-distribution PMF checks, large parameters and state rollback.
// UNSUPPORTED: c++98, c++03
#include <random>
#include <sstream>
#include <cassert>
#include <cmath>
#include <limits>
#include <climits>
struct CountingEngine {
    typedef std::mt19937::result_type result_type;
    std::mt19937 engine; unsigned calls;
    CountingEngine() : engine(79),calls(0) {}
    static constexpr result_type min() { return std::mt19937::min(); }
    static constexpr result_type max() { return std::mt19937::max(); }
    result_type operator()() { ++calls; return engine(); }
};
template<class D> void goodness(D distribution,const std::vector<double>& probabilities,double mean,double variance,double excess,int samples=90000) {
    const unsigned seeds[]={17,271,1009};
    std::vector<unsigned> counts(probabilities.size(),0);
    double sum=0,second=0;
    for (unsigned seed:seeds) {
        std::mt19937 engine(seed);
        for (int i=0;i<samples;++i) {
            auto value=distribution(engine); assert(value>=distribution.min() && value<=distribution.max());
            double centered=double(value)-mean; sum+=centered; second+=centered*centered;
            size_t bucket=static_cast<size_t>(value); if (bucket>=counts.size()) bucket=counts.size()-1;
            ++counts[bucket];
        }
    }
    const double total=samples*3;
    assert(std::abs(sum/total)<6*std::sqrt(variance/total));
    assert(std::abs(second/total-variance)<6*variance*std::sqrt((excess+2)/total));
    // Merge low-probability tail cells before the chi-square check.
    std::vector<double> expected,observed; double e=0,o=0;
    for (size_t i=0;i<counts.size();++i) {
        e+=total*probabilities[i]; o+=counts[i];
        if (e>=10) { expected.push_back(e); observed.push_back(o); e=o=0; }
    }
    if (e || o) { assert(!expected.empty()); expected.back()+=e; observed.back()+=o; }
    double chi=0; for(size_t i=0;i<expected.size();++i) { double d=observed[i]-expected[i]; chi+=d*d/expected[i]; }
    double degrees=double(expected.size()-1);
    assert(chi<degrees+8*std::sqrt(2*degrees)+24);
}
template<class D> void state(D d) {
    std::stringstream stream; stream.setf(std::ios::hex,std::ios::basefield); stream.fill('*'); stream.precision(2);
    auto flags=stream.flags(); stream<<d; assert(stream.flags()==flags && stream.fill()=='*' && stream.precision()==2);
    D copy; stream>>copy; assert(copy==d && stream.flags()==flags);
    std::mt19937 a(61),b(61); for(int i=0;i<100;++i) assert(d(a)==copy(b));
    d.reset(); assert(d==copy);
}
int main() {
    CountingEngine count;
    std::binomial_distribution<unsigned long long> zero(ULLONG_MAX,0),all(ULLONG_MAX,1);
    std::geometric_distribution<> immediate(1);
    assert(zero(count)==0 && all(count)==ULLONG_MAX && immediate(count)==0 && count.calls==0);
    std::binomial_distribution<> base(3,.25);
    int value=base(count,decltype(base)::param_type(10,.75)); assert(value>=0 && value<=10 && base.t()==3 && base.p()==.25);
    std::vector<double> bin(31); bin[0]=std::pow(1-.03125,30); double sum=bin[0];
    for(int i=0;i<29;++i) { bin[i+1]=bin[i]*(30-i)/(i+1)*.03125/(1-.03125); sum+=bin[i+1]; }
    bin.back()=(std::max)(0.,1-sum);
    double variance=30*.03125*(1-.03125);
    goodness(std::binomial_distribution<>(30,.03125),bin,30*.03125,variance,(1-6*.03125*(1-.03125))/variance);
    std::vector<double> central(151); central[0]=std::pow(.6,150); sum=central[0];
    for(int i=0;i<149;++i) { central[i+1]=central[i]*(150-i)/(i+1)*.4/.6; sum+=central[i+1]; }
    central.back()=(std::max)(0.,1-sum);
    goodness(std::binomial_distribution<>(150,.4),central,60,36,(1-6*.4*.6)/36,30000);
    std::vector<double> geo(41); sum=0;
    for(int i=0;i<40;++i) { geo[i]=.25*std::pow(.75,i); sum+=geo[i]; } geo.back()=1-sum;
    goodness(std::geometric_distribution<>(.25),geo,3,12,6+.25*.25/.75);
    std::vector<double> poisson(61); poisson[0]=std::exp(-20.); sum=poisson[0];
    for(int i=0;i<59;++i) { poisson[i+1]=poisson[i]*20/(i+1); sum+=poisson[i+1]; } poisson.back()=(std::max)(0.,1-sum);
    goodness(std::poisson_distribution<>(20),poisson,20,20,.05);
    std::mt19937 engine(53); double large_binomial=0,large_poisson=0,binomial_square=0,poisson_square=0;
    std::binomial_distribution<long long> big(1000000000000LL,.4);
    std::poisson_distribution<> many(100000000.);
    for(int i=0;i<1000;++i) { auto x=big(engine); assert(x>=0 && x<=big.t()); large_binomial+=x; binomial_square+=(double(x)-400000000000.)*(double(x)-400000000000.); auto y=many(engine); assert(y>=0); large_poisson+=y; poisson_square+=(double(y)-100000000.)*(double(y)-100000000.); }
    assert(std::abs(large_binomial/1000-400000000000.)<6*std::sqrt(1000000000000.*.4*.6/1000));
    assert(std::abs(large_poisson/1000-100000000.)<6*std::sqrt(100000000./1000));
    assert(std::abs(binomial_square/1000/(1000000000000.*.4*.6)-1)<.3);
    assert(std::abs(poisson_square/1000/100000000.-1)<.3);
    state(std::binomial_distribution<>(300,.125)); state(std::geometric_distribution<>(.25)); state(std::poisson_distribution<>(20));
    std::istringstream bad("7"); auto saved=base; bad>>base; assert(bad.fail() && base==saved);
    std::istringstream throwing("7"); throwing.exceptions(std::ios::failbit);
    try { throwing>>base; assert(false); } catch(const std::ios_base::failure&) {} assert(base==saved);
}
