# input: .JSON stream
# output: .JSON stream - optimized for a tracing based JIT  

import sys
import json
from form_blocks import form_blocks
from util import flatten
from tdce_hj424 import opt_dce

def opt_trace(func):

  # open profiling file
  profile_file = "tests/branch_profile.json" # Hardcoded for convenience
  with open(profile_file) as f:
    profile = json.load(f)
  #print(json.dumps(profile, indent=2))
  #print (len(profile["profile"]))
  
  profile_idx = 0
  # form blocks
  blocks = list(form_blocks(func['instrs']))

  iter_cnt = 0
  while (profile_idx < len(profile["profile"])):
    guard_arg = []
    recover_label = "recover"+str(iter_cnt)
    for block in blocks:
      element_idx = 0
      #print(block)
      for element in block:
        #print(json.dumps(element, indent=2))
        if profile_idx < len(profile["profile"]):
          if element == profile["profile"][profile_idx]:
            if (profile_idx == (iter_cnt * 3 + 0)):
              guard_arg = element["args"]
              #print("++++++++"+str(element["op"]))
              #print(guard_arg)
              del block[element_idx]
              # append speculate 
              block.append({"op":"speculate"})
            elif (profile_idx == (iter_cnt * 3 + 1)):
              del block[0]
              # append guard and commit 
              block.insert(0,{"args":guard_arg,"labels":recover_label,"op":"guard"})
              block.insert(-1,{"op":"commit"})
            else: # start recover
              # add label recover
              block.insert(0,{"label":recover_label})
            # update profile_idx
            profile_idx = profile_idx + 1
        else:
          pass
        element_idx = element_idx + 1
    iter_cnt = iter_cnt + 1
  # reassemable the function
  func['instrs'] = flatten(blocks)

def opt_funcs(func):
  opt_trace(func)
  opt_dce(func)

def local_opts():
  # Apply the change to all the functions in the input program.
  bril = json.load(sys.stdin)
  # iterate all functions
  for func in bril['functions']:
    opt_funcs(func)
  json.dump(bril, sys.stdout, indent=2, sort_keys=True)

# "real" main function
if __name__ == "__main__":
  local_opts()
