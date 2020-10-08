########################################################################
# Input: JSON stream of bril program in SSA format
# Output: Output the bril code 
########################################################################
import json
import sys

from cfg import block_map, add_terminators, add_entry, reassemble
from form_blocks import form_blocks

def convert_from_ssa(func):
  # label blocks
  blocks = block_map(form_blocks(func['instrs']))
  # add entry node if required
  add_entry(blocks)
  # add teminate node if required
  add_terminators(blocks)

  # Replace each phi-node.
  for block in blocks.values():
    # Insert copies for each phi.
    for instr in block:
      if instr.get('op') == 'phi':
        dest = instr['dest']
        type = instr['type']
        for i, label in enumerate(instr['labels']):
            var = instr['args'][i]
            # Insert a copy in the predecessor block, before the
            # terminator.
            pred = blocks[label]
            pred.insert(-1, {
                'op': 'id',
                'type': type,
                'args': [var],
                'dest': dest,
            })

    # Remove all phis.
    new_block = [i for i in block if i.get('op') != 'phi']
    block[:] = new_block
  # rebuild the updated blocks
  func['instrs'] = reassemble(blocks)


def from_ssa(bril):
  for func in bril['functions']:
    convert_from_ssa(func)
  return bril

# main function
if __name__ == '__main__':
  print(json.dumps(from_ssa(json.load(sys.stdin)), indent=2, sort_keys=True))
