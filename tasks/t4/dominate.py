########################################################################
# Input: JSON stream of bril program
# Output: print out one of the following choices
#         1. dom: dominator for the function
#         2. tree: the dominance tree
#         3. frontier: the dominance frontier
########################################################################
# reuse part of the functions from "dom.py"
# invoke the helper functions
import json
import sys

from form_blocks import form_blocks
from cfg import block_map, successors, add_terminators, add_entry

def map_inv(succ):
    """Invert a multimap.
    Given a successor edge map, for example, produce an inverted
    predecessor edge map.
    """
    out = {key: [] for key in succ}
    for p, ss in succ.items():
        for s in ss:
            out[s].append(p)
    return out

def postorder_helper(succ, root, explored, out):
    """Given a successor edge map, produce a list of all the nodes in
    the graph in postorder by appending to the `out` list.
    """
    if root in explored:
        return
    explored.add(root)

    for s in succ[root]:
        postorder_helper(succ, s, explored, out)
    out.append(root)

def postorder(succ, root):
    out = []
    postorder_helper(succ, root, set(), out)
    return out


def intersect(sets):
    sets = list(sets)
    if not sets:
        return set()
    out = set(sets[0])
    for s in sets[1:]:
        out &= s
    return out


def get_dom(succ, entry):
  pred = map_inv(succ)
  nodes = list(reversed(postorder(succ, entry)))  # Reverse postorder.
  # init dom 
  dom = {v: set(nodes) for v in succ}
  # print(dom)

  changed = True
  while (changed):
    changed = False
    # traverse every block
    for node in nodes:
      # dom for current node
      pred_list = []
      for pred_node in pred[node]:
        pred_list.append(dom[pred_node])
      new_dom = intersect(pred_list)
      new_dom.add(node)
      # update dom dic
      if dom[node] != new_dom:
        dom[node] = new_dom
        changed = True
      else:
        changed = False

  return dom

def dom_tree(dom):
  # Get the blocks strictly dominated by a block strictly dominated by
  # a given block.
  dom_inv = map_inv(dom)
  # strict dominate
  # one line implementation, learnt from example
  dom_inv_strict = {a: {b for b in bs if b != a}
                    for a, bs in dom_inv.items()}
  #dom_inv_strict = {}
  #for block_label, dom_inv_block_list in dom_inv.items():
  #  dom_inv_set = set()
  #  for dom_inv_block in dom_inv_block_list:
  #    if block_label != dom_inv_block:
  #      pass
  #    else:
  #      dom_inv_set.add(block_label)
  #  dom_inv_strict[block_label] = dom_inv_set
  #print(dom_inv_strict)
  dom_inv_strict_2x = {a: set().union(*(dom_inv_strict[b] for b in bs))
                       for a, bs in dom_inv_strict.items()}
  return {
    a: {b for b in bs if b not in dom_inv_strict_2x[a]}
    for a, bs in dom_inv_strict.items()
  }

def dom_fronts(dom, succ):
  """Compute the dominance frontier, given the dominance relation.
  """
  # frontier: doesn't dominate but dominate one of the predecessors 
  dom_inv = map_inv(dom)

  fronts = {}
  for block in dom:
    # Find all successors of dominated blocks.
    domin_succs = set()
    for domin in dom_inv[block]:
        domin_succs.update(succ[domin])

    # frontier: not strictly dominated by the
    # current block.
    fronts[block] = [b for b in domin_succs
                        if b not in dom_inv[block] or b == block]

  return fronts


# take in mode, the default one is "dom"
def print_dom(bril, mode):
  for func in bril['functions']:
    # return labeled blocks
    blocks = block_map(form_blocks(func['instrs']))
    #for name, block in blocks.items():
    #  print(str(name)+":"+str(block))
    # add entry block if required
    add_entry(blocks)
    # add terminators to every block if required
    add_terminators(blocks)
    # get the successor of each block 
    succ = {name: successors(block[-1]) for name, block in blocks.items()}

    #for name, block in succ.items():
    #  print(str(name)+": "+str(block))
    dom = get_dom(succ, list(blocks.keys())[0])

    if mode == 'front':
      res = dom_fronts(dom, succ)
    elif mode == 'tree':
      res = dom_tree(dom)
    else:
      res = dom

    # Format as JSON for stable output.
    print(json.dumps(
      {k: sorted(list(v)) for k, v in res.items()},
      indent=2, sort_keys=True,
    ))

# main function
if __name__ == '__main__':
  print_dom(
    json.load(sys.stdin),
    'dom' if len(sys.argv) < 2 else sys.argv[1]
  )
