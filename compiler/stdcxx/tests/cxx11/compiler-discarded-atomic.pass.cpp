// Original regression: discarded atomic results followed by EH regions.
#include <cassert>

static int checked, caught, destroyed;
struct Lifetime {
    ~Lifetime() { ++destroyed; }
};
__attribute__((noinline)) static void check(long long actual, long long expected) {
    assert(actual==expected);
    ++checked;
}
__attribute__((noinline)) static void raise(int value) { throw value; }

template<class T> void exercise() {
    volatile T value=0;
    Lifetime outer;
    try {
        raise(7);
    } catch(int error) {
        assert(error==7);
        Lifetime handler;
#define CHECK_DISCARDED(operation, expected) operation; check(value,expected)
        CHECK_DISCARDED(__sync_fetch_and_add(&value,3),3);
        CHECK_DISCARDED(__sync_add_and_fetch(&value,2),5);
        CHECK_DISCARDED(__sync_fetch_and_sub(&value,1),4);
        CHECK_DISCARDED(__sync_sub_and_fetch(&value,1),3);
        CHECK_DISCARDED(__sync_fetch_and_or(&value,8),11);
        CHECK_DISCARDED(__sync_or_and_fetch(&value,4),15);
        CHECK_DISCARDED(__sync_fetch_and_and(&value,7),7);
        CHECK_DISCARDED(__sync_and_and_fetch(&value,3),3);
        CHECK_DISCARDED(__sync_fetch_and_xor(&value,1),2);
        CHECK_DISCARDED(__sync_xor_and_fetch(&value,3),1);
        CHECK_DISCARDED(__sync_fetch_and_nand(&value,3),-2);
        CHECK_DISCARDED(__sync_nand_and_fetch(&value,7),-7);
        value=1;
        CHECK_DISCARDED(__sync_val_compare_and_swap(&value,1,9),9);
        CHECK_DISCARDED(__sync_bool_compare_and_swap(&value,9,11),11);
        CHECK_DISCARDED(__sync_val_compare_and_swap(&value,0,5),11);
        CHECK_DISCARDED(__sync_bool_compare_and_swap(&value,0,5),11);
        CHECK_DISCARDED(__sync_lock_test_and_set(&value,1),1);
        CHECK_DISCARDED(__sync_lock_release(&value),0);
        if (sizeof(T)==8) {
            const T high=T(1LL<<40);
            CHECK_DISCARDED(__sync_fetch_and_or(&value,high),high);
            CHECK_DISCARDED(__sync_xor_and_fetch(&value,3),high+3);
            CHECK_DISCARDED(__sync_and_and_fetch(&value,high),high);
            CHECK_DISCARDED(__sync_lock_test_and_set(&value,0),0);
        }
        // Consumed results still bind to their own intrinsic return register.
        assert(__sync_fetch_and_add(&value,2)==0);
        assert(__sync_add_and_fetch(&value,1)==3);
        assert(__sync_val_compare_and_swap(&value,3,5)==3);
        assert(__sync_bool_compare_and_swap(&value,5,7));
        assert(!__sync_bool_compare_and_swap(&value,5,9));
        assert(value==7);
#undef CHECK_DISCARDED
        // The next call's exception region must retain its unwind cleanups.
        __sync_add_and_fetch(&value,1);
        raise(13);
    }
}
int main() {
    try { exercise<int>(); assert(false); }
    catch(int error) { assert(error==13); ++caught; }
    try { exercise<long long>(); assert(false); }
    catch(int error) { assert(error==13); ++caught; }
    assert(checked==40 && caught==2 && destroyed==4);
}
