# Helper functions

import itertools

def form_instr_dict(file_name):
  # open the file
  f_ptr = open(file_name, "r")
  if (not f_ptr):
    print("! Can't open file: "+file_name)

  instr_list = {}
  line = f_ptr.readline()
  while line:
    instr_list.append(line)
    # read new line
    line = f_ptr.readline()

  # close the file
  f_ptr.close()

  return instr_list

def instr_list_to_file(file_name, instr_list):
  # open the file
  f_ptr = open(file_name, "w")
  if (not f_ptr):
    print("! Can't open file: "+file_name)

  for line in instr_list:
    f_ptr.write(line)

  # close the file
  f_ptr.close()


# copied from the example
def flatten(ll):
  """Flatten an iterable of iterable to a single list.
  """
  return list(itertools.chain(*ll))
