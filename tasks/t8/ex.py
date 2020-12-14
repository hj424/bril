import z3

def solver(phi):
  s = z3.Solver()
  s.add(phi)
  s.check()
  return s.model()

if __name__ == '__main__':
  """
  formula = (z3.Int('x') / 7 == 6)
  print(solver(formula))
  """
  """
  # bit vectors that supports shifting
  y = z3.BitVec('y', 8)
  print(solver(y << 3 == 40))
  """
  """
  # solving equations
  z = z3.Int('z')
  n = z3.Int('n')
  print(solver(z3.ForAll([z], z * n == z)))
  """
  x = z3.BitVec('x', 8)
  slow_expr = x * 2

  h = z3.BitVec('h', 8) # ??
  fast_expr = x << h

  goal = z3.ForAll([x], slow_expr == fast_expr)
  print(solver(goal))
  


