// Original provider for C++98/C++11 stream-failure ABI interoperability checks.
#include <sstream>
extern "C" void legacy_stream_failure() {
    std::istringstream input("bad"); input.exceptions(std::ios::failbit);
    int value; input>>value;
}
