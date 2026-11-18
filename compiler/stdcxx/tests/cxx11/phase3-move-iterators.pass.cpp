// Original coverage for insertion adapters with move-only deque elements.
#include <algorithm>
#include <deque>
#include <iterator>
#include <memory>
#include <cassert>

int main() {
    std::deque<std::unique_ptr<int> > values;
    auto back = std::back_inserter(values);
    auto front = std::front_inserter(values);
    std::unique_ptr<int> a(new int(2)), b(new int(1)), c(new int(3));
    *back++ = std::move(a);
    *front++ = std::move(b);
    auto middle = std::inserter(values, values.begin() + 1);
    *middle++ = std::move(c);
    assert(!a && !b && !c);
    assert(values.size() == 3);
    assert(*values[0] == 1 && *values[1] == 3 && *values[2] == 2);
    auto result = std::rotate(values.begin(), values.begin() + 1, values.end());
    assert(result == values.begin() + 2);
    assert(*values[0] == 3 && *values[1] == 2 && *values[2] == 1);
    assert(std::rotate(values.begin(), values.begin(), values.end()) == values.end());
    assert(std::rotate(values.begin(), values.end(), values.end()) == values.begin());
}
