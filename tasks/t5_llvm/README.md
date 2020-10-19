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

Source: https://github.com/sampsyo/llvm-pass-skeleton.git </br>

# Add a specific pass to LLVM compiler

After building the llvm-pass-skeleton, please use the following command to run the this pass with the examples available in the "tests" folder. </br>

This simple pass will find the integer multiplication (IMUL) from the source code, then call the runtime library to print out a message and also accumulating the total number of IMUL of each run. </br>

## Run the program with this simple pass
```javascript
  // compile source code
  $ clang -Xclang -load -Xclang build/skeleton/libSkeletonPass.* -c tests/vec_mul.c 
  // compile runtime library
  $ cc -c rtlib.c
  // link the file and generate the executable file
  $ cc rtlib.o vec_mul.o -o exe
  // execute it
  $ ./ext
```

## Sample output
```javascript
  Integer multiply detected! Total # of accumulated IMUL is 1.
  Integer multiply detected! Total # of accumulated IMUL is 2.
  Integer multiply detected! Total # of accumulated IMUL is 3.
  Integer multiply detected! Total # of accumulated IMUL is 4.
  Integer multiply detected! Total # of accumulated IMUL is 5.
  Integer multiply detected! Total # of accumulated IMUL is 6.
  Integer multiply detected! Total # of accumulated IMUL is 7.
  Integer multiply detected! Total # of accumulated IMUL is 8.
  Integer multiply detected! Total # of accumulated IMUL is 9.
  Integer multiply detected! Total # of accumulated IMUL is 10.
```
