Program Synthesis
=========================================================

This design is a naive program generator from a specification file. Currently, it will only generate simple kernels with the supported operators. <br />

## File list
1. ex.py: example code of learning how to use z3 for building this program generator.
2. kernel_generator.py: main script for generating the kernel code with the given specification.
3. spec.txt: specification read by the kernel generator.

## Detailed explanation
As a naive script, the kernel generator will only generate the code with two inputs and one output. Currently, the supported operators are "add", "sub", "mul", "div", "shl", and "shr".i <br/> 

The program specification should also follow the strict constrains: the first two line are the name of two inputs, then the third line is the name of supported operator, and the fourth line should be the name of the output. <br/> 

Thus this design is just a simple implementation of exploring the idea of program synthesis which can automatically generate code with the provided specification.<br/>

The script will first read the specification and then construct the equation, then verify the generated code using random tests. Finally it will output the generated Python code in the terminal. <br/>

## Run the code
```javascript
  $ python kernel_generator.py
```

## Sample outputs
```javascript
  Read spec file: spec.txt
  Gernerate kernel code...
  c = x * y
  Verify the gernerate kernel code...
  Done!
  The generated Python code: 

  if __name__ == '__main__':
    c = x * y

```

