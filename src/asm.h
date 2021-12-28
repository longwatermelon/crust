#ifndef ASM_H
#define ASM_H

#include "node.h"
#include "scope.h"

struct Asm
{
    char *data;
    char *root;

    struct Scope *scope;

    size_t lc;
    size_t stack_size;
};

struct Asm *asm_alloc();
void asm_free(struct Asm *as);

char *asm_gen_root(struct Asm *as, struct Node *node);
void asm_gen_expr(struct Asm *as, struct Node *node);

void asm_gen_function_def(struct Asm *as, struct Node *node);
void asm_gen_return(struct Asm *as, struct Node *node);

void asm_gen_variable_def(struct Asm *as, struct Node *node);
void asm_gen_store_string(struct Asm *as, struct Node *node);
void asm_gen_add_to_stack(struct Asm *as, struct Node *node);

void asm_gen_function_call(struct Asm *as, struct Node *node);

void asm_gen_builtin_print(struct Asm *as, struct Node *node);

struct Node *asm_eval_node(struct Asm *as, struct Node *node);
void asm_append_str(char **dst, char *src);

#endif

