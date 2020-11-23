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


# Learn from the reference code 
Reference: https://github.com/malhar1995/BasicLICM.git </br>
In LLVM, there are many member function we can leverate to simplfy our pass. Thus, I search online and find a good reference of implementing the basic LICM. </br>
I first read through the reference code, then reuse the skeleton of the runOnLoop top function. And starts to implement the sub-functions like preOrder, isLoopInvariant, safeToHoist, and LICM. Finally I make a comparison with the reference, and learn how to simplify the implementation by using LLVM member functions like the reference code, that is why the code in this repo is nearly the same as the reference. </br>
