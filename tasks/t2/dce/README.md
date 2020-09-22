Dead Code Elimination (DCE)
=========================================================

This Python based implementation supports the following optimization: <br />
(1) Delete duplicated assignment to the same variable
(2) Find the unused local assignment and removes them


## Run the code
```javascript
  $ bril2json < tests/simple.bril | python3 tdce_hj424.py | bril2txt
  $ bril2json < tests/simple.bril | python3 tdce_hj424.py | brili -p
```

## Sample outputs
```javascript
  $ bril2json < tests/simple.bril | brili -p

  6
  total_dyn_inst: 5

  $ bril2json < tests/simple.bril | python3 tdce_hj424.py | brili -p

  6
  total_dyn_inst: 4
```


```javascript
  $ bril2json < tests/reassign.bril | brili -p

  42
  total_dyn_inst: 3

  $ bril2json < tests/reassign.bril | python3 tdce_hj424.py | brili -p

  42
  total_dyn_inst: 2
```
