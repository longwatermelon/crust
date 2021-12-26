#ifndef ASM_H
#define ASM_H

#include "node.h"

char *asm_gen_root(struct Node *node);
char *asm_gen(struct Node *node);

char *asm_gen_function_def(struct Node *node);
char *asm_gen_return(struct Node *node);

#endif

