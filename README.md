# Crust

Crust programming language

I don't have any experience with x86 assembly, everything I know about it is from writing this compiler, so the generated code is probably not the best.

# Runtime dependencies
* as (GNU assembler)
* ld (GNU linker)

# Building
```
git clone https://github.com/longwatermelon/crust
cd crust
make
make stdlib
```

# Todo
* Rewrite builtin function pront to print variable length strings
* More standard library functions
* Loops
* Real integer type that can hold a reasonable range of numbers
* Else if / else

# Usage
## Keywords
* `fn`: Declare / define a function.
* `let`: Define a variable. (No variable declarations yet)
* `struct`: Declare / define a struct.

## Primitive types
* `int`: Integer type.
* `str`: String type.

## Hello world in crust
```
include "stdio";

fn main() -> int {
  print("Hello, world!");
  return 0;
};
```

`include "stdio"` is the equivalent of `#include <stdio.h>` in C. Can't print text without the standard print function.

To compile this program, run `./crust file`, file being the file that contains the code to be compiled. The executable will be named `a.out` by default.

Running `./a.out` will print `Hello`, because I still need to write the standard library, and without a string length function, I can't print variable length strings. Therefore, I decided all strings should be exactly 5 characters long. If the string length is less than 5, it is classified as UB, and if it is greater than 5, all extra characters will be cut off.

## Comments
Only felt like implementing full line comments, aka `//` in C.
```
fn main() -> int {
  // this is a comment.
  return 0;
};
```

## Variables
```
let globalVar: int = 5;

fn main() -> int {
  let localVar: str = "lmaooooo";
  let var: int = 2;
  return 0;
};
```

## Arithmetic
The four basic math operators (+ - * /) are implemented. Order of operations is not, and parentheses are not implemented either. Order of operations was braindead and completely arbitrary anyways.
```
include "stdio";

fn main() -> int {
  let a: int = 1 + 1;
  let b: int = a + 1;
  print(b);
  return 0;
};
```

Compiling and running this program prints nothing, because I need to write a standard library (I'm not going to) that is capable of converting integers to strings. Instead, the only way to enjoy crust's arithmetic experience is to print out the return value of the program. Modify the program:
```
fn main() -> int {
  let a: int = 1 + 1;
  let b: int = a + 1;
  return b;
};
```

`echo $?` should print out `3` after compiling this program.

Now guess the return value of this program. -1, right?
```
fn main() -> int {
  return 1 - 1 - 1;
};
```

Get jebaited nerd, integers are crippled and basically useless in crust, because I haven't figured out how to store integers of a reasonable size in assembly. The return value of this program is 255. Contributions are welcome.

## User defined functions
```
include "stdio";

fn user_defined_function() -> void {
  print("test\n");
};

fn main() -> int {
  user_defined_function();
  return 0;
};
```

Compiling this program will give an error, because underscores are not a recognized character by the lexer. It would be an easy fix, but I think it's funnier when people are forced to use garbage naming conventions such as camelCase.

![image](https://user-images.githubusercontent.com/73869536/158003281-a61cc309-678a-42b5-addc-05a2441c3400.png)

Modify the code:
```
fn userDefinedFunction() -> void {
  print("test\n");
};

fn main() -> int {
  userDefinedFunction();
  return 0;
};
```

Compiling and running this program will print `test`.

Some other function features
```
include "stdio";

fn paramFunc(param: int, string: str) -> void {
  print(string);
};

fn returnFunc(i: int) -> int {
  return i + 1;
};

fn main() -> int {
  paramFunc(1, "test\n");
  return returnFunc(1);
};
```

Compiling and running should print `test`, and `echo $?` should print `2`.

## Structs
```
include "stdio";

struct A {
  a: int,
  b: str
};

fn main() -> int {
  let a: A = A{ 1, "test\n" };
  print(a.b);
  return a.a;
};
```

Compiling and running this program will print `test` and `echo $?` will print `1`.

If you know any programming language, you should be able to grasp how structs work easily from this example.

Nested structs
```
include "stdio";

struct A { a: int, b: str };
struct B { a: str, b: A, c: int };
struct C { a: B };
struct D { a: int, b: C };

fn main() -> int {
  let var: D = D{ 1, C{ B{ "test\n", A{ 1, "test\n" }, 2 } } };
  return var.b.a.c;
};
```

This program will return `2`.

## Multiple files
Crust organizes its files similar to how C does it; header and source files. Headers contain declarations, and source contains definitions.

Example:
```
+
|
+--main.crust
|
+--source.crust
|
+--header
```

header:
```
fn prount(s: str) -> void;
```

source.crust:
```
include "header";
include "stdio";
fn prount(s: str) -> void {
  print(s);
};
```

main.crust:
```
include "header";
fn main() -> int {
  prount("test\n");
  return 0;
};
```

Compile with `./crust main.crust source.crust`. Running `./a.out` will print `test`.

## Control flow statements
Only if statements are implemented.

```
include "stdio";

fn main() -> int {
  let a: int = 1;
  
  if a == 1 {
    print("text\n");
  };
  
  return 0;
};
```

This program should print `text`.

## Inline assembly
Really only works if you know how the language works internally, so you probably shouldn't use it.
```
fn main() -> int {
  let a: str = "str \n";
  // this doesn't mean anything, just an example of how to use inline assembly.
  asm "movl $", idof a, ", %ecx";
  return 0;
};
```

## Compiler flags
`-o`: Specify output executable name. Example: `crust main.crust -o out`

`-S`: Keep generated assembly files. They will have the same name as the source file, with an extension of `.s` instead of `.crust`.

`-W`: Warning configuration flag. The flag to disable a warning will be displayed when said warning is printed:

![image](https://user-images.githubusercontent.com/73869536/158004277-94377035-72cc-4f11-b65e-0b0d5dacabd3.png)

To disable these warnings, you would use `crust main.crust -Wno-dead-code -Wno-unused-variable`. To enable warnings that were disabled by default, remove the `no-` from the flag, e.g.: `-Wdead-code` or `-Wunused-variable`.

`-L`: Add a library search path. Works the same as `-L` in gcc.

`-I`: Add an include search path. Works the same as `-I` in gcc. **Always** include the `/` at the end of the include path, or else the directory won't be included.

`-l`: Link to a static library. Works the same as `-l` in gcc.

`--obj`: Don't compile into the final executable, just object files. Same as `-c` in gcc.
