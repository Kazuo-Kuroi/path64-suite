// Original failed static-initializer cleanup, ordering and contended retry.
// UNSUPPORTED: c++98, c++03
#include <cassert>
#include <cstdlib>
#include <pthread.h>
#include <sched.h>
#include "static-retry-shared.h"
#ifndef STATIC_RETRY_CONSUMER_ONLY
int shared_retry_attempts;
int shared_retry_initialize() { if (++shared_retry_attempts==1) throw 17; return 71; }
extern "C" int static_retry_provider() { return shared_retry_value(); }
#endif
static int temporary_live, temporary_destroyed, outer_live, scalar_attempts;
struct Temporary {
    explicit Temporary(int) noexcept { ++temporary_live; }
    ~Temporary() { --temporary_live; ++temporary_destroyed; }
};
struct Outer {
    Outer() { ++outer_live; }
    ~Outer() { assert(temporary_live==0); --outer_live; }
};
int initialize_scalar(const Temporary&,const Temporary&) {
    assert(temporary_live==2 && outer_live==1);
    if (++scalar_attempts==1) throw 11;
    return 29;
}
int scalar_value() {
    Outer outer;
    static int value=initialize_scalar(Temporary(1),Temporary(2));
    assert(temporary_live==0);
    return value;
}
static int direct_attempts;
int direct_value() {
    static int value=(Temporary(3), ++direct_attempts==1 ? throw 73 : 77);
    return value;
}
static int argument_attempts, argument_destroyed, argument_calls;
struct ThrowingArgument {
    int value;
    ThrowingArgument() : value(1) { if (++argument_attempts==1) throw 31; }
    ~ThrowingArgument() { ++argument_destroyed; }
};
int initialize_argument(const ThrowingArgument&) { ++argument_calls; return 59; }
int argument_value() { static int value=initialize_argument(ThrowingArgument()); return value; }
static int destructor_attempts, destructor_calls;
struct ThrowingDestructor {
    int value;
    ThrowingDestructor() noexcept : value(1) {}
    ~ThrowingDestructor() noexcept(false) { if (++destructor_calls==1) throw 61; }
};
int initialize_destructor(const ThrowingDestructor&) { ++destructor_attempts; return 67; }
int destructor_value() { static int value=initialize_destructor(ThrowingDestructor()); return value; }
static int inner_attempts, outer_attempts, catch_attempts, handler_attempts;
int initialize_once(int& attempts,int error,int result) { if (++attempts==1) throw error; return result; }
int inner_value() { static int value=initialize_once(inner_attempts,13,31); return value; }
int nested_value() {
    static int value=(inner_value(),initialize_once(outer_attempts,19,37));
    return value;
}
int caught_value() {
    try {
        static int value=initialize_once(catch_attempts,23,41);
        return value;
    } catch(int error) { assert(error==23); return -1; }
}
int handler_value() {
    try { throw 5; }
    catch(int) {
        static int value=initialize_once(handler_attempts,7,43);
        return value;
    }
}
#if __cplusplus >= 201402L
static int lambda_attempts;
int lambda_value() {
    static auto value=[n=initialize_once(lambda_attempts,79,83)] { return n; };
    return value();
}
#endif
static int handled_attempts, statement_attempts, nested_statement_attempts;
int handled_statement_value() {
    static int value=({
        try { throw 3; } catch(int n) { assert(n==3); }
        89;
    });
    return value;
}
int rethrow_statement_value() {
    static int value=({
        try { initialize_once(handled_attempts,97,101); } catch(int) { throw; }
        101;
    });
    return value;
}
int nested_statement_value() {
    static int value=({
        Temporary temporary(1);
        static int inner=initialize_once(nested_statement_attempts,103,107);
        initialize_once(statement_attempts,109,inner+6);
    });
    return value;
}
static int arrived, arguments, argument_cleanup_done, object_attempts;
static int members, objects_destroyed, failures;
struct ThreadArgument {
    int generation;
    ThreadArgument() noexcept : generation(__sync_add_and_fetch(&arguments,1)) {
        if (generation>1) assert(__sync_add_and_fetch(&argument_cleanup_done,0)==1);
    }
    ~ThreadArgument() {
        if (generation==1) {
            for (int i=0;i<64;++i) sched_yield();
            __sync_add_and_fetch(&argument_cleanup_done,1);
        }
    }
};
struct Member {
    Member() noexcept { assert(__sync_add_and_fetch(&members,1)==1); }
    ~Member() { __sync_sub_and_fetch(&members,1); }
};
struct ThreadObject {
    Member member;
    int ready;
    explicit ThreadObject(const ThreadArgument&) : ready(0) {
        if (__sync_add_and_fetch(&object_attempts,1)==1) {
            for (int i=0;i<64;++i) sched_yield();
            throw 47;
        }
        ready=53;
    }
    ~ThreadObject() { ++objects_destroyed; }
};
ThreadObject& threaded_value() { static ThreadObject value((ThreadArgument())); return value; }
static void* worker(void*) {
    __sync_add_and_fetch(&arrived,1);
    while (__sync_add_and_fetch(&arrived,0)!=8) sched_yield();
    try { assert(threaded_value().ready==53); }
    catch(int error) { assert(error==47); assert(__sync_add_and_fetch(&failures,1)==1); assert(threaded_value().ready==53); }
    return 0;
}
static void final_check() { assert(objects_destroyed==1 && members==0); }
int main() {
    try { static_retry_provider(); assert(false); } catch(int n) { assert(n==17); }
    assert(shared_retry_value()==71 && static_retry_provider()==71 && shared_retry_attempts==2);
    try { scalar_value(); assert(false); } catch(int n) { assert(n==11); }
    assert(outer_live==0 && temporary_live==0 && temporary_destroyed==2);
    assert(scalar_value()==29 && outer_live==0 && temporary_live==0 && temporary_destroyed==4);
    assert(scalar_value()==29 && scalar_attempts==2 && temporary_destroyed==4);
    try { direct_value(); assert(false); } catch(int n) { assert(n==73); }
    assert(temporary_live==0);
    assert(direct_value()==77 && direct_attempts==2 && temporary_live==0);
    try { argument_value(); assert(false); } catch(int n) { assert(n==31); }
    assert(argument_destroyed==0 && argument_calls==0);
    assert(argument_value()==59 && argument_attempts==2 && argument_destroyed==1 && argument_calls==1);
    try { destructor_value(); assert(false); } catch(int n) { assert(n==61); }
    assert(destructor_value()==67 && destructor_calls==2 && destructor_attempts==2);
    try { nested_value(); assert(false); } catch(int n) { assert(n==13); }
    try { nested_value(); assert(false); } catch(int n) { assert(n==19); }
    assert(nested_value()==37 && inner_attempts==2 && outer_attempts==2);
    assert(caught_value()==-1 && caught_value()==41 && catch_attempts==2);
    try { handler_value(); assert(false); } catch(int n) { assert(n==7); }
    assert(handler_value()==43 && handler_attempts==2);
#if __cplusplus >= 201402L
    try { lambda_value(); assert(false); } catch(int n) { assert(n==79); }
    assert(lambda_value()==83 && lambda_value()==83 && lambda_attempts==2);
#endif
    assert(handled_statement_value()==89);
    try { rethrow_statement_value(); assert(false); } catch(int n) { assert(n==97); }
    assert(rethrow_statement_value()==101 && handled_attempts==2);
    try { nested_statement_value(); assert(false); } catch(int n) { assert(n==103); }
    try { nested_statement_value(); assert(false); } catch(int n) { assert(n==109); }
    assert(nested_statement_value()==113 && nested_statement_attempts==2 && statement_attempts==2);
    assert(temporary_live==0);
    assert(std::atexit(final_check)==0);
    pthread_t threads[8];
    for (int i=0;i<8;++i) assert(pthread_create(threads+i,0,worker,0)==0);
    for (int i=0;i<8;++i) assert(pthread_join(threads[i],0)==0);
    assert(failures==1 && object_attempts==2 && arguments==2);
    assert(argument_cleanup_done==1 && members==1 && objects_destroyed==0);
    assert(threaded_value().ready==53);
}
