#ifndef ASM_H
#define ASM_H

#include "node.h"
#include "scope.h"
#include "args.h"

struct Asm
{
    char *data;
    char *root;

    struct Scope *scope;

    size_t lc;
    size_t stack_size;

    struct Args *args;
};

struct Asm *asm_alloc(struct Args *args);
void asm_free(struct Asm *as);

void asm_gen_expr(struct Asm *as, struct Node *node);

void asm_gen_function_def(struct Asm *as, struct Node *node);
void asm_gen_return(struct Asm *as, struct Node *node);

void asm_gen_variable_def(struct Asm *as, struct Node *node);
// Add a string label to the data section.
void asm_gen_store_string(struct Asm *as, struct Node *node);
// Add data to stack
void asm_gen_add_to_stack(struct Asm *as, struct Node *node);

void asm_gen_function_call(struct Asm *as, struct Node *node);

void asm_gen_assignment(struct Asm *as, struct Node *node);

void asm_gen_builtin_print(struct Asm *as, struct Node *node);

// Get assembly representation of a node (x(%ebp), $.LCx, $x, %ebx, etc.)
char *asm_str_from_node(struct Asm *as, struct Node *node);
char *asm_str_from_int(struct Asm *as, struct Node *node);
char *asm_str_from_str(struct Asm *as, struct Node *node);
char *asm_str_from_var(struct Asm *as, struct Node *node);
char *asm_str_from_param(struct Asm *as, struct Node *node);
char *asm_str_from_function_call(struct Asm *as, struct Node *node);

#endif

