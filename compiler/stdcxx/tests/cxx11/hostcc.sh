#!/bin/sh
# The host clang++ with the suite's stdcxx headers and runtime libraries,
# in place of psclang: when a test is right with this and wrong with
# pathCC -fpsclang, the bug is in psclang (clang2whirl or the back end),
# not in the library.
#   PATH64_STAGE=BUILD/Xcompiler hostcc.sh -std=c++11 t.cc -o t
: "${PATH64_STAGE:?set PATH64_STAGE to BUILD/Xcompiler}"
I=$(ls -d "$PATH64_STAGE"/include/*/ | head -1)
L="$PATH64_STAGE/lib/x8664/64"
exec clang++ -nostdinc++ -nostdlib++ -isystem "$I" -isystem "$I/stl" \
    -isystem "$I/stl/ansi" '-D__STDCXX_CONFIG=<__stl_config-x86_64.h>' \
    -D_RWSTD_POSIX_THREADS -w "$@" "$L/libstl.a" "$L/libcxxrt.a" \
    "$L/libmv.a" "$L/libmpath.a" "$L/libpscrt.a" "$L/libeh.a" \
    -lpthread -ldl -lm
