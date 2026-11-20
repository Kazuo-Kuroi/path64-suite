// Original coverage for callee-saved registers restored by exception unwinding.
#include <cassert>
#if defined(__x86_64__)
#ifndef UNWIND_CONSUMER_ONLY
__attribute__((noinline)) void unwind_register_provider() {
    asm volatile("mov $11, %%rbx; mov $12, %%r12; mov $13, %%r13;"
                 "mov $14, %%r14; mov $15, %%r15"
                 : : : "rbx", "r12", "r13", "r14", "r15");
    throw 7;
}
#else
void unwind_register_provider();
#endif
#ifndef UNWIND_PROVIDER_ONLY
int main() {
    register long a asm("rbx") = 101;
    register long b asm("r12") = 102;
    register long c asm("r13") = 103;
    register long d asm("r14") = 104;
    register long e asm("r15") = 105;
    asm volatile("" : "+r"(a), "+r"(b), "+r"(c), "+r"(d), "+r"(e));
    try { unwind_register_provider(); assert(false); }
    catch (int value) { assert(value == 7); }
    asm volatile("" : "+r"(a), "+r"(b), "+r"(c), "+r"(d), "+r"(e));
    assert(a == 101 && b == 102 && c == 103 && d == 104 && e == 105);
}
#endif
#else
int main() { }
#endif
