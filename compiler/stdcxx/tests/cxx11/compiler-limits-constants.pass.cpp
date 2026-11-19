// Original psclang ABI regression: reference the library's namespaced constants.
#include <limits>
int main() {
    volatile double nan = std::numeric_limits<double>::quiet_NaN();
    volatile double tiny = std::numeric_limits<double>::denorm_min();
    return nan == nan || tiny == 0;
}
