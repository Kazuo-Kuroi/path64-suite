// Original provider compiled separately for mixed-compiler guard checks.
#include "static-retry-shared.h"
int shared_retry_attempts;
int shared_retry_initialize() { if (++shared_retry_attempts==1) throw 17; return 71; }
extern "C" int static_retry_provider() { return shared_retry_value(); }
