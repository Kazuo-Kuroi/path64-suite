// Original C++98-compatible checks for public flags and internal radix state.
#include <sstream>
#include <iostream>
#include <iomanip>
#include <cassert>
int main() {
    const std::ios::fmtflags initial=std::ios::skipws|std::ios::dec;
    std::stringstream s;
    assert(s.flags()==initial);
    assert(s.setf(std::ios::showbase)==initial);
    assert(s.flags()==(initial|std::ios::showbase));
    assert(s.flags(std::ios::hex|std::ios::uppercase)==(initial|std::ios::showbase));
    assert(s.flags()==(std::ios::hex|std::ios::uppercase));
    s << 255; assert(s.str()=="FF");
    s.str(""); s.clear(); s << std::oct << 63; assert(s.str()=="77");
    s.str(""); s.clear(); s << std::dec << -123; assert(s.str()=="-123");
    s.str("0xff 077 23"); s.clear(); s >> std::skipws >> std::setbase(0);
    int a=0,b=0,c=0; s>>a>>b>>c; assert(a==255 && b==63 && c==23);
    s.str(""); s.clear();
    s << std::setbase(3) << std::showbase << 10;
    assert(s.str()=="101");
    s.str("101"); s.clear(); s >> std::setbase(3) >> a; assert(a==10);
    s.flags(initial); assert(s.flags()==initial);
    s.setf(std::ios::nolock|std::ios::nolockbuf);
    assert((s.flags() & (std::ios::nolock|std::ios::nolockbuf))==
                       (std::ios::nolock|std::ios::nolockbuf));
    s.unsetf(std::ios::nolock|std::ios::nolockbuf);
    assert(s.flags()==initial);
    const bool sync=std::cout._C_is_sync();
    const std::ios::fmtflags old=std::cout.flags();
    std::cout.setf(std::ios::hex,std::ios::basefield);
    assert(std::cout._C_is_sync()==sync);
    std::cout.flags(old); assert(std::cout.flags()==old && std::cout._C_is_sync()==sync);
}
