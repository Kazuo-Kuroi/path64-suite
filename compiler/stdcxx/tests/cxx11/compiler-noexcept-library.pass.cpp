// Original regression: system-header catch ASTs with EH generation disabled.
// UNSUPPORTED: c++98, c++03
// PATH64_FLAGS: -fno-exceptions
#ifdef __EXCEPTIONS
# error this regression requires exception-disabled compilation
#endif
#include <random>
#include <regex>
#include <sstream>
#include <cassert>
int main() {
    std::random_device device; (void)device();
    std::mt19937 engine(7); std::normal_distribution<double> normal;
    (void)normal(engine);
    std::stringstream stream; stream<<42; int value=0; stream>>value; assert(value==42);
    std::cmatch match;
    assert(std::regex_search("xx aaa yy",match,std::regex("(a+)")) && match.str(1)=="aaa");
}
