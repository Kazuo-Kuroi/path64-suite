// Original oversized-growth checks must throw, with well-formed diagnostics.
// UNSUPPORTED: c++98, c++03
#include <vector>
#include <deque>
#include <list>
#include <limits>
#include <stdexcept>
#include <cassert>
#include <cstring>
template<class Operation> void length_error(Operation operation) {
    try { operation(); assert(false); }
    catch(const std::length_error& error) { assert(std::strlen(error.what())>0); }
}
int main() {
    const std::size_t huge=(std::numeric_limits<std::size_t>::max)();
    length_error([&] { std::vector<int> values(huge); });
    std::vector<int> values(2,7);
    length_error([&] { values.insert(values.begin(),huge,8); });
    length_error([&] { values.resize(huge); });
    assert(values.size()==2 && values.front()==7);
    std::vector<bool> bits(2,true);
    length_error([&] { bits.insert(bits.begin(),huge,false); });
    length_error([&] { bits.resize(huge); });
    assert(bits.size()==2 && bits.front());
    length_error([&] { std::deque<int> queue(huge); });
    std::deque<int> queue(2,9);
    length_error([&] { queue.assign(huge,10); });
    length_error([&] { queue.insert(queue.begin(),huge,11); });
    assert(queue.size()==2 && queue.front()==9);
    length_error([&] { std::list<int> nodes(huge); });
    std::list<int> nodes(2,12);
    length_error([&] { nodes.insert(nodes.cbegin(),huge,13); });
    assert(nodes.size()==2 && nodes.front()==12);
}
