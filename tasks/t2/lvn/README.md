Local Value Numbering (LVN)
=========================================================

This Python based implementation supports the following optimization: <br />
(1) Basic version of Common Subexpression Elimination (CSE)
(2) Copy propagation
(3) CSE exploiting commutativity

Also, it will first perform the LVN optimization, then call the DCE optimiation implemented in the file "tdce_hj424.py".

## Run the code
```javascript
  $ bril2json < tests/idchain.bril | python3 lvn_hj424.py | bril2txt
  $ bril2json < tests/idchain.bril | python3 lvn_hj424.py | brili -p
```

## Sample outputs
```javascript
  $ bril2json < tests/idchain.bril | python3 lvn_hj424.py | bril2txt

  @main {
    x: int = const 4;
    print x;
  }
```


```javascript
  $ bril2json < tests/redundant.bril | python3 lvn_hj424.py | bril2txt

  @main {
    a: int = const 4;
    b: int = const 2;
    sum1: int = add a b;
    prod: int = mul sum1 sum1;
    print prod;
  }
```
