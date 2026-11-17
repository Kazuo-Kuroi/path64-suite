#!/bin/sh
# Downloads the libc++ 8.0.1 sources (its tests are the conformance check;
# NCSA or MIT license) into DIR (default ./libcxx-8.0.1).
set -e
dir=${1:-libcxx-8.0.1}
url=https://github.com/llvm/llvm-project/releases/download/llvmorg-8.0.1/libcxx-8.0.1.src.tar.xz
mkdir -p "$dir"
curl -sSLf "$url" | tar -xJ -C "$dir" --strip-components=1
echo "LIBCXX_SRC=$(cd "$dir" && pwd)"
