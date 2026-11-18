// Original coverage for nontrivial result objects and their ABI return area.
#include <cassert>
#include <functional>

struct Result {
    static int alive;
    int value;
    int* self;
    explicit Result(int v) : value(v), self(&value) { ++alive; }
    Result(const Result& rhs) : value(rhs.value), self(&value) { ++alive; }
    Result(Result&& rhs) noexcept : value(rhs.value), self(&value) { ++alive; }
    ~Result() { assert(self == &value); --alive; }
    void check(int expected) const { assert(self == &value && value == expected); }
};
int Result::alive;
Result named_result(int value) { Result local(value); return local; }
Result forwarded_result(int value) { return named_result(value); }
Result selected_result(bool first, int value) {
    return first ? named_result(value) : Result(value + 1);
}
struct Factory {
    virtual Result make(int value) = 0;
    virtual ~Factory() { }
};
struct Prefix { long payload; virtual ~Prefix() { } };
struct DerivedFactory : Prefix, Factory {
    Result make(int value) { return named_result(value); }
    Result operator()(int value) { return Result(value); }
};
int main() {
    {
        Result first = named_result(1);
        first.check(1);
        Result second = forwarded_result(2);
        second.check(2);
        Result (*function)(int) = &named_result;
        Result indirect = function(3);
        indirect.check(3);
        DerivedFactory factory;
        Factory* base = &factory;
        Result virtual_result = base->make(4);
        virtual_result.check(4);
        Result (Factory::*member)(int) = &Factory::make;
        Result member_result = (base->*member)(5);
        member_result.check(5);
        Result operator_result = factory(6);
        operator_result.check(6);
        Result chosen = selected_result(true, 7);
        chosen.check(7);
        Result alternate = selected_result(false, 7);
        alternate.check(8);
        std::function<Result(int)> wrapped = named_result;
        Result erased = wrapped(9);
        erased.check(9);
    }
    assert(Result::alive == 0);
}
