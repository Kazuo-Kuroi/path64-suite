// Open compiler/runtime ABI issue: psclang refers to an unmangled constant,
// while the GCC-built library and host Clang use its __rw namespace name.
#include <limits>
int main() {
    volatile double nan = std::numeric_limits<double>::quiet_NaN();
    volatile double tiny = std::numeric_limits<double>::denorm_min();
    return nan == nan || tiny == 0;
}
