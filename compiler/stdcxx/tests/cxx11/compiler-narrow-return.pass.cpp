// Original x86-64 scalar-return ABI checks; provider modes allow cross-compiler tests.
#include <cassert>
extern "C" {
bool low_false(); bool low_true();
signed char low_signed(); unsigned char low_unsigned();
short low_short(); unsigned short low_ushort();
}
#ifndef NARROW_CONSUMER_ONLY
extern "C" __attribute__((noinline)) bool low_false() {
    bool result; __asm__ volatile("movl $0x123400, %%eax" : "=a"(result)); return result;
}
extern "C" __attribute__((noinline)) bool low_true() {
    bool result; __asm__ volatile("movl $0x123401, %%eax" : "=a"(result)); return result;
}
extern "C" __attribute__((noinline)) signed char low_signed() {
    signed char result; __asm__ volatile("movl $0x1234f9, %%eax" : "=a"(result)); return result;
}
extern "C" __attribute__((noinline)) unsigned char low_unsigned() {
    unsigned char result; __asm__ volatile("movl $0x123481, %%eax" : "=a"(result)); return result;
}
extern "C" __attribute__((noinline)) short low_short() {
    short result; __asm__ volatile("movl $0x1234ff85, %%eax" : "=a"(result)); return result;
}
extern "C" __attribute__((noinline)) unsigned short low_ushort() {
    unsigned short result; __asm__ volatile("movl $0x1234f123, %%eax" : "=a"(result)); return result;
}
#endif
#ifndef NARROW_PROVIDER_ONLY
int main() {
    assert(!low_false());
    assert(int(low_true()) == 1);
    assert(low_signed() == -7 && low_unsigned() == 0x81);
    assert(low_short() == -123 && low_ushort() == 0xf123);
    bool (*volatile indirect)() = low_false;
    assert(!indirect());
    indirect = low_true;
    assert(int(indirect()) == 1);
}
#endif
