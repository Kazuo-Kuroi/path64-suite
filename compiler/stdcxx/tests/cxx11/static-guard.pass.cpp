// Original guard ABI, contended initialization and explicit abort/retry checks.
// UNSUPPORTED: c++98, c++03
#include <cstdint>
#include <cassert>
#include <pthread.h>
#include <sched.h>
extern "C" int __cxa_guard_acquire(volatile std::int64_t*);
extern "C" void __cxa_guard_release(std::int64_t*);
extern "C" void __cxa_guard_abort(std::int64_t*);
struct State {
    std::int64_t guard;
    int arrived,winners,value;
};
static void* contender(void* raw) {
    State& state=*static_cast<State*>(raw);
    __sync_add_and_fetch(&state.arrived,1);
    while (__sync_add_and_fetch(&state.arrived,0)!=8) sched_yield();
    if (__cxa_guard_acquire(&state.guard)) {
        __sync_add_and_fetch(&state.winners,1);
        assert(*reinterpret_cast<unsigned char*>(&state.guard)==0);
        for (int i=0;i<64;++i) sched_yield();
        state.value=73;
        __cxa_guard_release(&state.guard);
    }
    assert(state.value==73);
    assert(*reinterpret_cast<unsigned char*>(&state.guard)==1);
    return 0;
}
static int constructions;
struct Value {
    int ready;
    Value() : ready(0) {
        __sync_add_and_fetch(&constructions,1);
        for (int i=0;i<128;++i) sched_yield();
        ready=91;
    }
};
Value& local_value() { static Value value; return value; }
static int callers;
static void* local_contender(void*) {
    __sync_add_and_fetch(&callers,1);
    while (__sync_add_and_fetch(&callers,0)!=8) sched_yield();
    for (int i=0;i<100;++i) assert(local_value().ready==91);
    return 0;
}
int main() {
    std::int64_t guard=0;
    assert(__cxa_guard_acquire(&guard)==1);
    assert(*reinterpret_cast<unsigned char*>(&guard)==0);
    __cxa_guard_abort(&guard);
    assert(__cxa_guard_acquire(&guard)==1);
    __cxa_guard_release(&guard);
    assert(*reinterpret_cast<unsigned char*>(&guard)==1);
    for (int i=0;i<100;++i) assert(__cxa_guard_acquire(&guard)==0);
    for (int round=0;round<32;++round) {
        State state={0,0,0,0};
        pthread_t threads[8];
        for (int i=0;i<8;++i) assert(pthread_create(threads+i,0,contender,&state)==0);
        for (int i=0;i<8;++i) assert(pthread_join(threads[i],0)==0);
        assert(state.winners==1 && state.value==73);
    }
    pthread_t threads[8];
    for (int i=0;i<8;++i) assert(pthread_create(threads+i,0,local_contender,0)==0);
    for (int i=0;i<8;++i) assert(pthread_join(threads[i],0)==0);
    assert(constructions==1);
}
