#!/usr/bin/env bash
set -x
set -e

mkdir -p build

LLVM_COV="${LLVM_COV:-llvm-cov}"
LLVM_GCOV=$(realpath misc/llvm_gcov.sh)
chmod +x misc/llvm_gcov.sh

# clang -lm -coverage -g -std=gnu99 -DNDEBUG -DUFBX_VOID_ASSERT -DUFBX_DEV=1 -DUFBX_REGRESSION=1 -DUFBXT_THREADS=1 -pthread ufbx.c test/runner.c -o build/cov-runner

clang++ -coverage -c -g -std=c++11 -DUFBX_COVERAGE_LCOV -DUFBX_DEV=1 -DUFBX_REGRESSION=1 -DUFBXT_THREADS=1 -x c++ ufbx.c -o build/ufbx-cov.o
clang -lm -coverage -g -std=gnu99 -DUFBX_COVERAGE_LCOV -DUFBX_DEV=1 -DUFBX_REGRESSION=1 -DUFBXT_THREADS=1 -pthread build/ufbx-cov.o test/runner.c -o build/cov-runner

build/cov-runner -d data
$LLVM_COV gcov ufbx runner -b
lcov --directory . --base-directory . --gcov-tool $LLVM_GCOV --config-file misc/lcovrc --capture -o coverage.lcov
