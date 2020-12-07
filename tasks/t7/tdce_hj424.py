# input: .JSON stream
# output: .JSON stream - optimized with DCE, removes the unused duplicated assignments

import sys
import json
from form_blocks import form_blocks
from util import flatten

def opt_dce(func):
 
  # global optimization
  blocks = list(form_blocks(func['instrs']))

  # collect useful variable list
  used_dest = []
  for block in blocks:
    for element in block:
      if "args" in element:
        for op in element["args"]:
          used_dest.append(op)
  # remove duplicated element
  used_dest = list(dict.fromkeys(used_dest))

  # iterate all blocks
  # delete unused assignment
  for block in blocks:
    opt_done = False
    while(not opt_done):
      num_instr = len(block) 
      # check all instructions and delete the unused ones
      for element in block:
        del_instr = True
        if "dest" in element:
          for op in used_dest:
            if op == element["dest"]:
              del_instr = False
        else:
          del_instr = False
        # delete the useless instructions
        if del_instr:
          #print("+++"+str(element))
          block.remove(element)
      # if no changes, optimization done
      if num_instr == len(block):
        opt_done = True

  # iterate all blocks
  # delete duplicated assignment
  # remove duplicated assignment for each block
  for block in blocks:
    opt_done = False
    while(not opt_done):
      num_instr = len(block) 
      # build the assigned but not used dictionary
      assigned_not_used = {}
      element_ptr = 0
      for element in block:
        # check the assigned list
        if "dest" in element:
          for key,value in assigned_not_used.items():
            if element["dest"] == key:
              # delete the old assignment
              #print("==="+str(block[value]))
              del block[value]
              break
        # add new assignment
        if "dest" in element:
          assigned_not_used[element["dest"]] = element_ptr 
        # update the assigned but not used dicionary
        if "args" in element:
          for op in element["args"]:
            keys = []
            for key,value in assigned_not_used.items():
              if op == key:
                keys.append(key)
                break
          for key in keys:
            del assigned_not_used[key]
        # update element pointer
        element_ptr = element_ptr + 1

      # if no changes, optimization done
      if num_instr == len(block):
        opt_done = True

  # Reassemble the function.
  func['instrs'] = flatten(blocks)

def opt_func(func):
  opt_dce(func)

def local_opts():
  # copied from example
  # Apply the change to all the functions in the input program.
  bril = json.load(sys.stdin)
  # iterate all functions
  for func in bril['functions']:
    opt_func(func)
  json.dump(bril, sys.stdout, indent=2, sort_keys=True)

# "real" main function
if __name__ == "__main__":
  local_opts()
  
