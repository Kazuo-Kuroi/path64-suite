// Original linkage regression; optional modes support two-compiler ABI checks.
#include <cassert>
namespace left {
extern "C++" { extern int value; extern const long constant; }
extern "C" { extern int shared_c; }
}
namespace right {
extern "C++" { extern int value; extern const long constant; }
extern "C" { extern int shared_c; }
}
struct Record { static int member; };
#if !defined(LINKAGE_CONSUMER_ONLY)
namespace left { extern "C++" { int value = 11; extern const long constant = 13; } }
namespace right { extern "C++" { int value = 17; extern const long constant = 19; } }
extern "C" { int shared_c = 23; }
int Record::member = 29;
#endif
#if !defined(LINKAGE_PROVIDER_ONLY)
static int constructed;
struct Holder { int value; Holder(int x): value(x) { ++constructed; } };
int guarded() { static Holder holder(31); return holder.value; }
template<class T> T local_values(T parameter) { T local = parameter + 1; return local; }
int main() {
    assert(left::value == 11 && right::value == 17);
    assert(left::constant == 13 && right::constant == 19);
    assert(&left::value != &right::value);
    assert(&left::shared_c == &right::shared_c && left::shared_c == 23);
    assert(Record::member == 29);
    assert(local_values(41) == 42);
    assert(guarded() == 31 && guarded() == 31 && constructed == 1);
}
#endif
