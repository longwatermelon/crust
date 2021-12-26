#ifndef ASM_H
#define ASM_H

#include "node.h"
#include "scope.h"

struct Asm
{
    char *data;
    char *root;

    struct Scope *scope;
};

struct Asm *asm_alloc();
void asm_free(struct Asm *as);

char *asm_gen_root(struct Asm *as, struct Node *node);
void asm_gen(struct Asm *as, struct Node *node);

void asm_gen_function_def(struct Asm *as, struct Node *node);
void asm_gen_return(struct Asm *as, struct Node *node);

void asm_gen_variable_def(struct Asm *as, struct Node *node);
void asm_gen_variable(struct Asm *as, struct Node *node);

void asm_gen_function_call(struct Asm *as, struct Node *node);

void asm_gen_builtin_print(struct Asm *as, struct Node *node);

#endif

