Data Flow Analysis
=========================================================

This Python based implementation performs the data flow analysis for multiple simple global analysis <br />
(1) defined: basic analysis that just accumulate all the currently-defined variables <br />
(2) live: live variable analysis <br />
(3) cprop: a simple constant propagation pass <br/>
(4) reachdef: a simple reach definition pass <br />

I start to look into the provided script written in Python and decide to make an extension to it, i.e., writing a new pass to enforce the current data flow analysis. Because I think this is the common method to do reserach related to compilers, i.e.,we usually start from an existing compiling flow, adding new passes to further optimize it. And that is why I rename the file called "de_ex.py". <br/>

In the reachdef pass, it will output a dictionary to notify the valid the definition reaches the use of this block. Specifically, the dictionary is built with the instruction as the kay and the variable name as the value. <br />

I also use the turnt to help me quickly switching between analyses. <br />

## Run the code
```javascript
  $ turnt -vp -a reachdef tests/cond.bril
  $ turnt -vp -a live tests/fact.bril
```

