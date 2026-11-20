// Original provider for mixed-compiler pooled-list ownership checks.
#include <list>
extern "C" std::list<int>* make_transfer_list() {
    std::list<int>* result=new std::list<int>;
    result->push_back(37); result->push_back(41); return result;
}
