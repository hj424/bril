Static Single Assignment
=========================================================

These Python based impletation solves the follow problem:<br />
(1) dominate.py: print out one of the following choices <br />
a. dom: dominator for the function <br />
b. tree: the dominance tree <br />
c. frontier: the dominance frontier<br />
(2) to_ssa.py: take the JSON stream and output the bril code in SSA format<br />
(3) from_ssa.py: take the SSA JSON stream and output hte bril code <br/>

I build the dominate.py by referring the "dom.py" example, and as for the to_ssa and from_ssa code, I first understand the examples and reuse most of the functions from "to_ssa.py" and "from_ssa.py" example.

## Run the code
```javascript
  $ bril2json < tests/while.bril | python3 dominate.py tree 
  $ bril2json < tests/if.bril | python3 to_ssa.py 
  $ bril2json < tests/loop-ssa.bril | python3 from_ssa.py 
```

