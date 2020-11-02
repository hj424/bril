# llvm-pass-skeleton

A completely useless LLVM pass.
It's for LLVM 3.8.

Build:

    $ cd llvm-pass-skeleton
    $ mkdir build
    $ cd build
    $ cmake ..
    $ make
    $ cd ..

Run:

    $ clang -Xclang -load -Xclang build/skeleton/libSkeletonPass.* something.c

# Add the Loop-Invariant Code Motion (LICM) in LLVM 

After building the llvm-pass-skeleton, please use the following command to run the this pass with the examples available in the "tests" folder. </br>

## Run the program with this simple pass
```javascript
  // compile source code
  $ clang -Xclang -load -Xclang build/skeleton/libSkeletonPass.* tests/vec_add.c
  // run it
  $ ./a.out
```

## Sample output
```javascript
  LICM Optimization...
  DONE!
```

Reference: https://github.com/malhar1995/BasicLICM.git </br>
