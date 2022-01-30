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
* Implement returning struct from function
* Implement struct copying
* Loops, if statements, etc.
* List data type

