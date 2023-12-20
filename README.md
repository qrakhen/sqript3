# sqript

#### ...actually, it's sqript 3.0 by now... :D
## Faster, Better, Stronger

Now for a quick an dirty introduction:

written in full-on legacy, non-ISO-Compliant pure C code that uses about 20 to 40kB as a binary depending on your OS.
A down-to machine-code pre-compiled interpreter that is much faster than [insert most other interpreters here]
Few steps away of compiling into assembly, and this is actually something I can see someone be working with for fun, in the feature.
A small example file for usage is inside the root src folder. (test.sqr)

General idea is very scattered, but will form over time.

#### Usage:
Either sqript_c.exe ./yourFile.sqr 
or
sqript_c.exe, using the CLI

## Basics:
### Variable declaration
```js
 <: *~ a <~ 5;					// alias for var and =
 <: var b = 'test';
 <: *~ c <~ [ 1, 2, 3, 10 ];
 <: :: a;						// shorthand for print
 :> 5
 <: print b;
 :> test
 <: print c[2];
 :> 3
```

### Functions
```js
// <: shorthand for return, fq for funq or function
 <: fq fib(n) { if (n < 2) <: n; <: fib(n-1) + fib(n-2); }
 <: print fib(24)
```

### Everything is References
```js
 <: *~ a <~ [ 1, 5, 10, [ 33, 66*2, time ], length ]; // these are 2 native fq calls
 <: ::a[3][2]();
 :> 0.1827
 <: ::a[4][0]("hello world");
 :> 11
 <: :: typeof a[3];
 :> array<any>[3]
 <: :: typoef a[4];
 :> <native fn>
```

### Operators
#### Declare
```js
*~ or var or *Type~

*~ x <~ 0xf;
var y <~ 5;
*Number~ z <~ 0.43;
*<T>~ generic <~ ...;
```

#### Auto Assign
```js
<~ or =

*~ x <~ y;
*~ var = y;
```

#### Assign by Reference
```js
& or <&

*~ x <~ &y;
*~ x <& y;
```

#### Assign by Value
```js
<| or <~ |

*~ x <| y;
*~ x <~ |y;
```

#### Assign by Reference
```js
& or <&

*~ x <~ &y;
*~ x <& y;
```

#### List Operators
```js
<+ add right to left
+> add left to right
<- remove from right (and add to left)
-> remove from left (and add to right)
:* pop from list

list <+ 5;
list[5] ->
list[3] +> list2
*~ x <~ list:* //equal to x <- list[:]
```

#### Return
```js
<: or return
```

### Classes
```js 
qlass Vector { 
	init(x, y, z) { 
		this:x <~ x;  // all of these work
		this.y = y;   // normie code works too! 
		.~:z <~ z;    // very unsure about syntax just yet
	}
	
	length() {
		<: sqrt(x*x + y*y + z*z);
	}		
}

*~ v <~ Vector(3, 5, 10);
:: v:length();
```

### Natives 
```js
 <: *~ t <~ time();
 <: fib(32);
 <: :: time() - t;
 :> 0.072ms // 500% faster than JS :^)
 ```

Sqript comes with 0 core modules so it's prettyn bare bone atm, but I'm definietely looking into making this big time - 
potentially even looking at Rust/Cpp for ~special tasks~.

I do wish all of you a great evening!
