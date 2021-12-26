#ifndef SCOPE_H
#define SCOPE_H

#include "node.h"

struct Scope
{
    struct Node **variable_defs;
    size_t variable_defs_size;
};

struct Scope *scope_alloc();
void scope_free(struct Scope *scope);

void scope_add_variable_def(struct Scope *scope, struct Node *node);
struct Node *scope_find_variable(struct Scope *scope, char *name);

#endif

