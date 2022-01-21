# Crust
Crust programming language

I don't have any experience with x86 assembly, everything I know about it is from writing this compiler, so the generated code is probably not the best.

# Runtime dependencies
* as
* ld
* cp

# Building
```
git clone https://github.com/longwatermelon/crust
cd crust
make
```

# Bugs
* Builtin function pront can only print strings 5 characters in length. Less is UB, more will cut off extra characters.
* Structs crash the compiler often when being passed between functions, a massive refactor may be necessary

