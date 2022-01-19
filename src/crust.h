#ifndef CRUST_H
#define CRUST_H

#include "args.h"

void crust_compile(struct Args *args);

struct Node *crust_gen_ast(struct Args *args);
char *crust_gen_asm(struct Node *root, struct Args *args);
void crust_assemble(char *as, struct Args *args);

#endif

