#ifndef CRUST_H
#define CRUST_H

#include "args.h"

void crust_compile(struct Args *args);
void crust_compile_file(struct Args *args, char *file);

struct Node *crust_gen_ast(struct Args *args, char *file);
struct Token **crust_tokenize(char *file, size_t *ntokens);

char *crust_gen_asm(struct Node *root, struct Args *args, bool main);

void crust_assemble(char *as, struct Args *args, char *file);
void crust_link(struct Args *args, char **files, size_t nfiles);

#endif

