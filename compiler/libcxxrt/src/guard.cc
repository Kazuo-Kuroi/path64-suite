/**
 * guard.cc: Functions for thread-safe static initialisation.
 *
 * Static values in C++ can be initialised lazily their first use.  This file
 * contains functions that are used to ensure that two threads attempting to
 * initialize the same static do not call the constructor twice.  This is
 * important because constructors can have side effects, so calling the
 * constructor twice may be very bad.
 *
 * Statics that require initialisation are protected by a 64-bit value.  Any
 * platform that can do 32-bit atomic test and set operations can use this
 * value as a low-overhead lock.  Because statics (in most sane code) are
 * accessed far more times than they are initialised, this lock implementation
 * is heavily optimised towards the case where the static has already been
 * initialised.  
 */
#include <stdint.h>
#include <pthread.h>
#include <sched.h>
#include <stdlib.h>

// The ABI completion byte is the first byte in memory on either endian.
// Keep the spin lock in the other 32-bit word so a pending initializer never
// looks complete to compiler-generated first-byte fast paths.
static int32_t completion_mask()
{
    const int one=1;
    return *(const char*)&one ? 1 : 0x01000000;
}

static int32_t* guard_lock(volatile int64_t* guard)
{
    return (int32_t*)guard+1;
}

static bool guard_complete(volatile int64_t* guard)
{
    int32_t mask=completion_mask();
    return __sync_bool_compare_and_swap_4((int32_t*)guard,mask,mask);
}

extern "C" int __cxa_guard_acquire(volatile int64_t* guard)
{
    if (guard_complete(guard)) return 0;
    int32_t* lock=guard_lock(guard);
    while (!__sync_bool_compare_and_swap_4(lock,0,1)) {
        if (guard_complete(guard)) return 0;
        sched_yield();
    }
    // A waiter can win the lock after another thread published completion.
    // It must release the lock even though it will not initialize anything.
    if (guard_complete(guard)) {
        if (!__sync_bool_compare_and_swap_4(lock,1,0)) abort();
        return 0;
    }
    return 1;
}

extern "C" void __cxa_guard_abort(int64_t* guard)
{
    if (!__sync_bool_compare_and_swap_4(guard_lock(guard),1,0)) abort();
}

extern "C" void __cxa_guard_release(int64_t* guard)
{
    // Full-barrier publication of completed construction before unlocking.
    if (!__sync_bool_compare_and_swap_4((int32_t*)guard,0,completion_mask())) abort();
    __cxa_guard_abort(guard);
}
