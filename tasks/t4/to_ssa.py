########################################################################
# Input: JSON stream of bril program
# Output: Output the bril code in ssa format
########################################################################
import json
import sys

from collections import defaultdict

from cfg import block_map, successors, add_terminators, add_entry, reassemble
from form_blocks import form_blocks
from dominate import get_dom, dom_fronts, dom_tree, map_inv

def def_blocks(blocks):
  """Get a map from variable names to defining blocks.
  """
  out = defaultdict(set)
  for name, block in blocks.items():
    for instr in block:
      if 'dest' in instr:
        out[instr['dest']].add(name)
  return dict(out)


def get_phis(blocks, df, defs):
  """Find where to insert phi-nodes in the blocks.
  Produce a map from block names to variable names that need phi-nodes
  in those blocks. (We will need to generate names and actually insert
  instructions later.)
  """
  phis = {b: set() for b in blocks}
  for v, v_defs in defs.items():
    v_defs_list = list(v_defs)
    for d in v_defs_list:
      for block in df[d]:
        # Add a phi-node...
        if v not in phis[block]:
          # ..unless we already did.
          phis[block].add(v)
          if block not in v_defs_list:
            v_defs_list.append(block)
  return phis

def ssa_rename(blocks, phis, succ, domtree, args):
  stack = defaultdict(list, {v: [v] for v in args})
  phi_args = {b: {p: [] for p in phis[b]} for b in blocks}
  phi_dests = {b: {p: None for p in phis[b]} for b in blocks}
  counters = defaultdict(int)

  def _push_fresh(var):
    fresh = '{}.{}'.format(var, counters[var])
    counters[var] += 1
    stack[var].insert(0, fresh)
    return fresh

  def _rename(block):
    # Save stacks.
    old_stack = {k: list(v) for k, v in stack.items()}

    # Rename phi-node destinations.
    for p in phis[block]:
      phi_dests[block][p] = _push_fresh(p)

    for instr in blocks[block]:
      # Rename arguments in normal instructions.
      if 'args' in instr:
        new_args = [stack[arg][0] for arg in instr['args']]
        instr['args'] = new_args

      # Rename destinations.
      if 'dest' in instr:
        instr['dest'] = _push_fresh(instr['dest'])

    # Rename phi-node arguments (in successors).
    for s in succ[block]:
      for p in phis[s]:
        if stack[p]:
          phi_args[s][p].append((block, stack[p][0]))

    # Recursive calls.
    for b in sorted(domtree[block]):
        _rename(b)

    # Restore stacks.
    stack.update(old_stack)

  entry = list(blocks.keys())[0]
  _rename(entry)

  return phi_args, phi_dests

def insert_phis(blocks, phi_args, phi_dests, types):
  for block, instrs in blocks.items():
    for dest, pairs in sorted(phi_args[block].items()):
      phi_node = {
        'op': 'phi',
        'dest': phi_dests[block][dest],
        'type': types[dest],
        'labels': [p[0] for p in pairs],
        'args': [p[1] for p in pairs],
      }
      instrs.insert(0, phi_node)

def get_types(func):
    # Silly way to get the type of variables. (According to the Bril
    # spec, well-formed programs must use only a single type for every
    # variable within a given function.)
    types = {arg['name']: arg['type'] for arg in func.get('args', [])}
    for instr in func['instrs']:
        if 'dest' in instr:
            types[instr['dest']] = instr['type']
    return types

def convert_to_ssa(func):
  # return labeled blocks
  blocks = block_map(form_blocks(func['instrs']))
  # add entry block if required
  add_entry(blocks)
  # add terminators to block if required
  add_terminators(blocks)
  # get the successor of each block
  succ = {name: successors(block[-1]) for name, block in blocks.items()}
  # get the predecessor of each block
  pred = map_inv(succ)
  # get dominator of each block
  dom = get_dom(succ, list(blocks.keys())[0])
  # get dominate frontier of each block
  df = dom_fronts(dom, succ)
  # varaible map to block label
  defs = def_blocks(blocks)
  # get function type
  types = get_types(func)
  # get argument name 
  arg_names = {a['name'] for a in func['args']} if 'args' in func else set()
  # find phi nodes 
  phis = get_phis(blocks, df, defs)
  # rename variables  
  phi_args, phi_dests = ssa_rename(blocks, phis, succ, dom_tree(dom),
                                   arg_names)
  # inserte phi nodes 
  insert_phis(blocks, phi_args, phi_dests, types)
  # rebuild the updated blocks
  func['instrs'] = reassemble(blocks)


def to_ssa(bril):
  for func in bril['functions']:
    convert_to_ssa(func)
  return bril

# main function
if __name__ == '__main__':
  print(json.dumps(to_ssa(json.load(sys.stdin)), indent=2, sort_keys=True))





