import z3
import lark
from random import seed
from random import random

seed("deadbeef")

GRAMMAR = """
?start: sum

?sum: term
  | sum "+" term        -> add
  | sum "-" term        -> sub

?term: item
  | term "*"  item      -> mul
  | term "/"  item      -> div
  | term ">>" item      -> shr
  | term "<<" item      -> shl

?item: NUMBER           -> num
  | "-" item            -> neg
  | CNAME               -> var
  | "(" start ")"

%import common.NUMBER
%import common.WS
%import common.CNAME
%ignore WS
""".strip()

def interp(tree, lookup):
  op = tree.data
  if op in ('add', 'sub', 'mul', 'div', 'shl', 'shr'):
    lhs = interp(tree.children[0], lookup)
    rhs = interp(tree.children[1], lookup)
    if op == 'add':
      return lhs + rhs
    elif op == 'sub':
      return lhs - rhs
    elif op == 'mul':
      return lhs * rhs
    elif op == 'div':
      return lhs / rhs
    elif op == 'shl':
      return lhs << rhs
    elif op == 'shr':
      return lhs >> rhs
  elif op == 'neg':
    sub = interp(tree.children[0], lookup)
    return -sub
  elif op == 'num':
    return int(tree.children[0])
  elif op == 'var':
    return lookup(tree.children[0])

def read_spec(filename):
  spec_info = []
  with open("spec.txt") as fp:
    lines = fp.readlines()
    for line in lines:
      spec_info.append(line.strip('\n')) 
  # return spec info
  return spec_info

# Hard coded for this naive code generator
def interp_ops(op_info):
  if op_info == 'add':
    return '+'
  elif op_info == 'sub':
    return '+'
  elif op_info == 'mul':
    return '*'
  elif op_info == 'div':
    return '/'
  elif op_info == 'shl':
    return '<<'
  elif op_info == 'shr':
    return '>>'
  # finish the program if receive unknown operator from spec
  print("Unknown operator!")
  quit()

def ref_res(op_info, op1, op2):
  if op_info == 'add':
    return op1 + op2
  elif op_info == 'sub':
    return op1 - op2
  elif op_info == 'mul':
    return op1 * op2
  elif op_info == 'div':
    return op1 / op2
  elif op_info == 'shl':
    return op1 << op2
  elif op_info == 'shr':
    return op1 >> op2
  # finish the program if receive unknown operator from spec
  print("Unknown operator!")
  quit()

def print_python_kernel_code(equ_full):
  print("if __name__ == '__main__':")
  print("  " + str(equ_full))

# main function
if __name__ == '__main__':
  # read spec
  filename = "spec.txt"
  print ("Read spec file: " + filename)
  spec_info = read_spec(filename)
  operator = interp_ops(spec_info[2])

  print ("Gernerate kernel code...")
  # form equation
  equ_compute = spec_info[0] + " " + operator + " " + spec_info[1]
  equ_full    = spec_info[3] + " = " + equ_compute
  print(equ_full)  

  print ("Verify the gernerate kernel code...")
  # verify the funtionality of the generated kernel
  parser = lark.Lark(GRAMMAR)
  tree = parser.parse(equ_compute)
 
  num_tests = 10
  env = {spec_info[0]: 2, spec_info[1]:-17}
  for i in range(num_tests):
    op1 = random()
    op2 = random()
    result_ref = ref_res(spec_info[2], op1, op2)
    env[spec_info[0]] = op1
    env[spec_info[1]] = op2
    #print(interp(tree, lambda v: env[v]))
    if (interp(tree, lambda v: env[v]) != result_ref):
      print("Generated kernel failed the functional verification...")
      print("Internal ERROR!")
    
  print ("Done!")
  print ("The generated Python code: \n")
  print_python_kernel_code(equ_full)






