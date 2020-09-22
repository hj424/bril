# input: .JSON stream
# output: .JSON stream - optimized with LVN
# Optimization: Common Subscription Elimination (CSE)
#               copy propagation
#               CSE expoiting commutativity

import sys
import json
from form_blocks import form_blocks
from util import flatten
from tdce_hj424 import opt_dce

def instr_type(instr):
  # receives one instruction
  # return 0 common operands: add sub ...
  # return 1 if it is a const instruction
  # return 2 if it is a copy instruction
  # return 3 if it is a print instruction
  value_type_tmp = 0

  if "op" in instr:
    if instr["op"] == "const":
      value_type_tmp = 1
    elif instr["op"] == "id":
      value_type_tmp = 2
    elif instr["op"] == "print":
      value_type_tmp = 3

  return value_type_tmp


def last_write(block):
  # receives a block of instructions
  # returns a list with boolean valus that denotes 
  # whether this instruction will be overwritten later
  last_write_list = []
  for i in range(len(block)):
    last_write_cur = True
    if "dest" in block[i]:
      for j in range(1, len(block)):
        if "dest" in block[j]:
          if block[i]["dest"] == block[j]["dest"]:
            last_write_cur = False
            break
    last_write_list.append(last_write_cur)

    return last_write_list

def opt_lvn(func):
 
  # global optimization
  blocks = list(form_blocks(func['instrs']))

  # list of dictionary, key: experssion; value: canonical variable name
  table = []
  # dictionary, key: variable name; value: row_idx in table
  var2num = {}
  # iterate all blocks
  for block in blocks:
    instr_ptr = 0
    instr_to_be_delete = [0]*len(block)
    for element in block:
      reuse = False
      reuse_idx = 0
      # value has been compute before
      value_cur = {}
      if "args" in element:
        for op in element["args"]:
          value_cur["args"] = op
      if "op" in element: 
        value_cur["op"] = element["op"]
        if element["op"] == "id" or element["op"] == "print":
          if element["args"][0] in var2num:
            if "op" in table[var2num[element["args"][0]]][0]:
              reuse_idx = var2num[element["args"][0]]
              if table[var2num[element["args"][0]]][0]["op"] == "id":
                 reuse = True
      if "value" in element: 
        value_cur["value"] = str(element["value"])
      #print(table)
      #print(value_cur)
      #=====================================================
      # check whether the value is available in the table 
      #=====================================================
      # same operation 
      for i in range(len(table)):
        # CSE - same expression
        if value_cur == table[i][0]:
          reuse = True
          reuse_idx = i
      if reuse:
        #print("reuse")
        if "dest" in element:
          # add new element to var2num 
          var2num[element["dest"]] = reuse_idx
        # replace value or delete it
        if instr_type(element) == 1: # const
          # mark this instruction and delete it later
          instr_to_be_delete[instr_ptr] = 1
          #del block[instr_ptr]
        elif instr_type(element) == 2: # id
          element.update({
            "args": table[reuse_idx][0]["args"],
          })
        elif instr_type(element) == 3: # print
          element.update({
            "args": table[reuse_idx][0]["args"],
          })
        else:
          # mark this instruction and delete it later
          instr_to_be_delete[instr_ptr] = 1
      else: # update the table
        if "dest" in element:
          # add new element to var2num
          var2num[element["dest"]] = len(table)
          # update table
          table.append([value_cur, element["dest"]])

      # update args in instr
      if instr_type(element) == 0:
        # two operands points to the same row
        if len(element["args"]) == 2:
          if var2num[element["args"][0]] == var2num[element["args"][1]]:
            idx = var2num[element["args"][0]]
            new_args = [table[idx][1]]*2
            element.update({
              "args": new_args,
            })

        else:
          print("Unsupported instructions...")

      # update instruction pointer
      instr_ptr = instr_ptr + 1

    # end of one block
    # delete useless instructions
    for i in range(len(block)):
      if instr_to_be_delete[i]:
        del block[i]


  # Reassemble the function.
  func['instrs'] = flatten(blocks)

def opt_func(func):
  opt_lvn(func)
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
  
