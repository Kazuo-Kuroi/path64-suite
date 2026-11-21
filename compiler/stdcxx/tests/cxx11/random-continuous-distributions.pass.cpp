// Original continuous-distribution CDF, moments, type and stream regressions.
// UNSUPPORTED: c++98, c++03
#include <random>
#include <sstream>
#include <cmath>
#include <cassert>

template<class D,class CDF> void cdf_bins(D d,CDF cdf) {
    const unsigned seeds[]={37,401,2029};
    const int n=30000; unsigned bins[10]={};
    for(unsigned seed:seeds) {
        std::mt19937 engine(seed);
        for(int i=0;i<n;++i) {
            auto x=d(engine); assert(x>=d.min() && x<=d.max());
            double p=cdf(double(x)); assert(p>=0 && p<=1);
            ++bins[p==1?9:int(p*10)];
        }
    }
    double chi=0;
    for(unsigned count:bins) { double diff=double(count)-3*n/10; chi+=diff*diff/(3*n/10); }
    assert(chi<75);
}
template<class D> void moments(D d,double mean,double variance,double fourth) {
    std::mt19937 engine(715); const int n=120000; double sum=0,second=0;
    for(int i=0;i<n;++i) { double x=double(d(engine))-mean; sum+=x; second+=x*x; }
    assert(std::abs(sum/n)<6*std::sqrt(variance/n));
    assert(std::abs(second/n-variance)<6*std::sqrt((fourth-variance*variance)/n));
}
template<class D> void state(D d,typename D::param_type alternate) {
    auto saved=d; std::mt19937 a(17),b(17); D other(alternate);
    for(int i=0;i<20;++i) assert(d(a,alternate)==other(b));
    assert(d==saved); d.reset(); assert(d==saved);
    std::stringstream stream; stream.setf(std::ios::hex,std::ios::basefield); stream.precision(2); stream.fill('*');
    auto flags=stream.flags(); stream<<d;
    assert(stream.flags()==flags && stream.precision()==2 && stream.fill()=='*');
    D copy; stream>>copy; assert(copy==d && stream.flags()==flags);
    std::mt19937 c(41),e(41); for(int i=0;i<20;++i) assert(copy(c)==d(e));
    std::istringstream bad("bad"); bad>>d; assert(bad.fail() && d==saved);
    std::istringstream throwing("bad"); throwing.exceptions(std::ios::failbit);
    try { throwing>>d; assert(false); } catch(const std::ios_base::failure&) {} assert(d==saved);
}
template<class R> void types() {
    state(std::weibull_distribution<R>(R(.5),R(1.234567890123456789L)),typename std::weibull_distribution<R>::param_type(2,3));
    state(std::extreme_value_distribution<R>(R(-2.123456789012345678L),R(.5)),typename std::extreme_value_distribution<R>::param_type(1,3));
    state(std::chi_squared_distribution<R>(R(.123456789012345678L)),typename std::chi_squared_distribution<R>::param_type(5));
    state(std::cauchy_distribution<R>(R(-1.234567890123456789L),R(2.5)),typename std::cauchy_distribution<R>::param_type(3,1));
    state(std::fisher_f_distribution<R>(R(.5),R(2.123456789012345678L)),typename std::fisher_f_distribution<R>::param_type(4,6));
    state(std::student_t_distribution<R>(R(2.123456789012345678L)),typename std::student_t_distribution<R>::param_type(5));
}
struct ZeroThenMidpoint {
    typedef unsigned result_type;
    unsigned calls;
    ZeroThenMidpoint() : calls(0) {}
    static constexpr unsigned min() { return 0; }
    static constexpr unsigned max() { return 0xffffffffU; }
    unsigned operator()() { return calls++?0x80000000U:0; }
};
int main() {
    ZeroThenMidpoint endpoint;
    float value=std::extreme_value_distribution<float>()(endpoint);
    assert(std::isfinite(value) && endpoint.calls==2);
    ZeroThenMidpoint zero;
    assert(std::weibull_distribution<float>()(zero)==0 && zero.calls==1);
    const double pi=std::acos(-1.);
    cdf_bins(std::weibull_distribution<>(2,3),[](double x){return -std::expm1(-(x/3)*(x/3));});
    cdf_bins(std::extreme_value_distribution<>(2,3),[](double x){return std::exp(-std::exp(-(x-2)/3));});
    cdf_bins(std::chi_squared_distribution<>(2),[](double x){return -std::expm1(-x/2);});
    cdf_bins(std::chi_squared_distribution<>(1),[](double x){return std::erf(std::sqrt(x/2));});
    cdf_bins(std::cauchy_distribution<>(2,3),[pi](double x){return .5+std::atan((x-2)/3)/pi;});
    cdf_bins(std::fisher_f_distribution<>(2,2),[](double x){return x/(1+x);});
    cdf_bins(std::fisher_f_distribution<>(4,4),[](double x){double z=x/(1+x);return z*z*(3-2*z);});
    cdf_bins(std::student_t_distribution<>(1),[pi](double x){return .5+std::atan(x)/pi;});
    cdf_bins(std::student_t_distribution<>(2),[](double x){return .5+x/(2*std::sqrt(2+x*x));});
    // Float and long-double samplers must preserve the same parameterized law.
    cdf_bins(std::weibull_distribution<float>(2,3),[](double x){return -std::expm1(-(x/3)*(x/3));});
    cdf_bins(std::student_t_distribution<long double>(2),[](double x){return .5+x/(2*std::sqrt(2+x*x));});
    double wmean=3*std::tgamma(1.5),wv=9-wmean*wmean;
    double wfourth=81*std::tgamma(3.)-4*wmean*27*std::tgamma(2.5)+6*wmean*wmean*9-3*std::pow(wmean,4);
    moments(std::weibull_distribution<>(2,3),wmean,wv,wfourth);
    double ev=9*pi*pi/6;
    moments(std::extreme_value_distribution<>(2,3),2+3*.5772156649015328606,ev,5.4*ev*ev);
    moments(std::chi_squared_distribution<>(5),5,10,5.4*100);
    moments(std::student_t_distribution<>(8),0,8./6,4.5*(8./6)*(8./6));
    double raw[5]={1};
    for(int r=1;r<=4;++r) raw[r]=raw[r-1]*2*(10+2*(r-1))/(20-2*r);
    double mean=raw[1],variance=raw[2]-mean*mean;
    moments(std::fisher_f_distribution<>(10,20),mean,variance,raw[4]-4*mean*raw[3]+6*mean*mean*raw[2]-3*std::pow(mean,4));
    types<float>(); types<double>(); types<long double>();
    std::weibull_distribution<> w(2,3),saved=w; std::istringstream truncated("7"); truncated>>w; assert(truncated.fail() && w==saved);
    std::wstringstream wide; std::chi_squared_distribution<long double> chi(.123456789012345678L),copy;
    wide<<chi; wide>>copy; assert(copy==chi);
}
