# TODOS

## compiler

File Structure:
```
meta:
	uint size_strings;
	uint size_qlosures;
	byte[32] extra;
qlosures:
	qlosure[size_qlosures];
strings:
	(uint, String)[size_strings];
program:
	Byte*
\EOF
```

## import export
runner.cs make runner nonglobal but param based
add Value* exports to module (which should be a runner imho, or a global Imports array)

## type safe stuff
mode_dyn, mode_strict, mode_const

### bitshifting
doesnt work at all atm since no type safety

### real integers (atm only doubles)
only with type safety

## array add does not work with indexes
like x[3] <+ 5; doesn't work, but *~ y <~ x[3]; then y <+ 5 does. something in digester, probably, stack is wrong

## native + method no work
length(string.split(' ')) -> error

# FIXED
#### run file doesnt work (run("core.sqr") for example) and i have 0 clues why.