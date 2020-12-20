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


# LLVM path for loop flattening - A naive implementation
This is a naive implementation of flattening the loop so that other loop related optimizations can be better performed. The idea is quite straightforward. For example, here is a C code with nested for loop.

```javascript
#define N 11
#define M 20

int main() {
  int res[M*N];
  for (int i = 0; i < N; i++) {
    for (int j = 0; j < M; j++) {
      res[i*M+j] = i*M+j;
    }
  }
}
```
After the loopfalttening pass, the nested loop will be flattened and the code will be like this:

```javascript
#define N 11
#define M 20

int main() {
  int res[M*N];
  for (int i = 0; i < N; i++) {
    res[i] = i;
  }
}
```

After building the llvm-pass-skeleton, please use the following command to run the this pass with the examples available in the "tests" folder. </br>

## Run the example with the script
This implementation is being tested under LLVM v11.0.0. To run the pass, please enter the directory with the "profile.sh" script, run the folloing in the command window:
```javascript
  $ source profile.sh
```

## Output from the terminal
```javascript
Default optimizations... 
Unnested loop, stop flattening. 
Default optimizations... 
Find nested loop...
Loop flattening running on nested loop: 
Exiting and latch block are different.
Loop bound: i32 11
Exiting and latch block are different.
Loop bound: i32 20
=============
Outer Loop IR:   %8 = load i32, i32* %4, align 4
Outer Loop IR:   %9 = icmp slt i32 %8, 11
Outer Loop IR:   br i1 %9, label %10, label %29
=============
Inner Loop IR:   %12 = load i32, i32* %5, align 4
Inner Loop IR:   %13 = icmp slt i32 %12, 20
Inner Loop IR:   br i1 %13, label %14, label %25
Loop at depth 1 containing: %7<header><exiting>,%10,%11,%25,%26<latch>,%14,%22
    Loop at depth 2 containing: %11<header><exiting>,%14,%22<latch>

Multiplication res: 30; 
    1|       |#include <stdio.h>
    2|       |#include <stdlib.h>
    3|       |
    4|      2|#define N 11
    5|    441|#define M 20
    6|       |
    7|      1|int main() {
    8|      1|  int init_val = 30;
    9|      1|  int res[M*N];
   10|      1|  LOOP:
   11|      2|  for (int i = 0; i < N; i++) {
   12|    221|    for (int j = 0; j < M; j++) {
   13|    220|      res[i*M+j] = init_val;
   14|    220|    }
   15|      1|  }
   16|      1|  printf("Multiplication res: %d; \n", res[0]);
   17|      1|  return 0;
   18|      1|}
```

### Explanation
Actually, it is hard to make the whole pass work. Thus I simplify the process by changing the loop bound of the nested loops. As we can see from the output, the inner loop bound has been changed to 220, which is the multiplication of ```N*M```, and the loop bound for the outer loop has been changed to 1. By doing this, we can explore other loop optimizations like loop unrolling inside the inner loop to get a better speedup compared with the rolled loop design.

## Reference
The loop flattening pass has just been merged into the official LLVM github repository (LLVM v12.0.0). Here is the [source code](https://llvm.org/doxygen/LoopFlatten_8cpp_source.html). Unlike my naive implementation, this pass can actually flatten the loop as shown in the following example:

```javascript
 // from nested loop:
 for (int i = 0; i < N; ++i)
   for (int j = 0; j < M; ++j)
     f(A[i*M+j]);
 // into one loop:
 for (int i = 0; i < (N*M); ++i)
   f(A[i]);
```

However, it also has some constraints which is elaborated in the [source code](https://llvm.org/doxygen/LoopFlatten_8cpp_source.html).
