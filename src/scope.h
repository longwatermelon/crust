#ifndef SCOPE_H
#define SCOPE_H

#include "node.h"

struct Scope
{
    struct ScopeLayer
    {
        struct Node **variable_defs;
        size_t variable_defs_size;

        struct Node **params;
        size_t nparams;
    } **layers;
    size_t nlayers;
    struct ScopeLayer *curr_layer;

    struct Node **function_defs;
    size_t function_defs_size;

    struct Node **struct_defs;
    size_t struct_defs_size;
};

struct Scope *scope_alloc();
void scope_free(struct Scope *scope);
struct ScopeLayer *layer_alloc();
void layer_free(struct ScopeLayer *layer);

void scope_add_variable_def(struct Scope *scope, struct Node *node);
void scope_add_function_def(struct Scope *scope, struct Node *node);
void scope_add_struct_def(struct Scope *scope, struct Node *node);

// Pass -1 for error_line to suppress errors if target is not found
struct Node *scope_find_variable(struct Scope *scope, struct Node *var, int err_line);
struct Node *scope_find_variable_struct_member(struct Scope *scope, struct Node *var, int err_line);
struct Node *scope_find_function(struct Scope *scope, char *name, int err_line);
struct Node *scope_find_function_def(struct Scope *scope, char *name, int err_line);
struct Node *scope_find_function_decl(struct Scope *scope, char *name, int err_line);
struct Node *scope_find_struct(struct Scope *scope, char *name, int err_line);

void scope_pop_layer(struct Scope *scope);
void scope_push_layer(struct Scope *scope);

// Copies all defs from s2 to s1
// TODO Copy all layers
void scope_combine(struct Scope *s1, struct Scope *s2);

#endif

