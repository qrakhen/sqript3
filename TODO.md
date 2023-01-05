# TODOS

## type safe stuff
mode_dyn, mode_strict, mode_const

### bitshifting
doesnt work at all atm since no type safety

### real integers (atm only doubles)
only with type safety

## array add does not work with indexes
like x[3] <+ 5; doesn't work, but *~ y <~ x[3]; then y <+ 5 does. something in digester, probably, stack is wrong