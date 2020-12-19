#!/bin/bash

/usr/local/opt/llvm/bin/clang -fprofile-instr-generate -fcoverage-mapping -Xclang -load -Xclang build/skeleton/libSkeletonPass.* tests/vec_add.c

LLVM_PROFILE_FILE="a.profraw" ./a.out
/usr/local/opt/llvm/bin/llvm-profdata merge -sparse a.profraw -o a.profdata
/usr/local/opt/llvm/bin/llvm-cov show ./a.out -instr-profile=a.profdata
