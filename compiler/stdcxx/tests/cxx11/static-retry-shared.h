// Original inline-static fixture for cross-compiler exception retry.
#ifndef PATH64_STATIC_RETRY_SHARED_H
#define PATH64_STATIC_RETRY_SHARED_H
extern int shared_retry_attempts;
int shared_retry_initialize();
inline int shared_retry_value() {
    static int value=shared_retry_initialize();
    return value;
}
extern "C" int static_retry_provider();
#endif
