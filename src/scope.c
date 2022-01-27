#include "scope.h"

#include <string.h>
#include <stdio.h>


struct Scope *scope_alloc()
{
    struct Scope *scope = malloc(sizeof(struct Scope));
    scope->layers = 0;
    scope->nlayers = 0;
    scope->curr_layer = 0;

    scope->function_defs = 0;
    scope->function_defs_size = 0;

    scope->struct_defs = 0;
    scope->struct_defs_size = 0;

    return scope;
}


void scope_free(struct Scope *scope)
{
    if (scope->layers)
    {
        for (size_t i = 0; i < scope->nlayers; ++i)
            layer_free(scope->layers[i]);

        free(scope->layers);
    }

    if (scope->function_defs)
        free(scope->function_defs);

    if (scope->struct_defs)
        free(scope->struct_defs);

    free(scope);
}


struct ScopeLayer *layer_alloc()
{
    struct ScopeLayer *layer = malloc(sizeof(struct ScopeLayer));
    layer->variable_defs = 0;
    layer->variable_defs_size = 0;

    layer->params = 0;
    layer->nparams = 0;

    return layer;
}


void layer_free(struct ScopeLayer *layer)
{
    if (layer->variable_defs)
        free(layer->variable_defs);

    // Params are never freed because params is a non owning pointer
    // pointing to memory owned by a node

    free(layer);
}


void scope_add_variable_def(struct Scope *scope, struct Node *node)
{
    scope->curr_layer->variable_defs = realloc(
        scope->curr_layer->variable_defs,
        sizeof(struct Node*) * ++scope->curr_layer->variable_defs_size
    );

    scope->curr_layer->variable_defs[scope->curr_layer->variable_defs_size - 1] = node;
}


void scope_add_function_def(struct Scope *scope, struct Node *node)
{
    scope->function_defs = realloc(scope->function_defs, sizeof(struct Node*) * ++scope->function_defs_size);
    scope->function_defs[scope->function_defs_size - 1] = node;
}


void scope_add_struct_def(struct Scope *scope, struct Node *node)
{
    scope->struct_defs = realloc(scope->struct_defs, sizeof(struct Node*) * ++scope->struct_defs_size);
    scope->struct_defs[scope->struct_defs_size - 1] = node;
}


struct Node *scope_find_variable(struct Scope *scope, struct Node *var)
{
    for (size_t layer = 0; layer < scope->nlayers; ++layer)
    {
        for (size_t i = 0; i < scope->layers[layer]->variable_defs_size; ++i)
        {
            struct Node *def = scope->layers[layer]->variable_defs[i];

            if (strcmp(def->variable_def_name, var->variable_name) == 0)
            {
                // TODO Rewrite to handle nested initializer lists
                if (var->variable_struct_member)
                {
                    struct Node *struct_def = scope_find_struct(scope,
                        node_type_from_node(def->variable_def_value, scope).struct_type);

                    for (size_t i = 0; i < struct_def->struct_members_size; ++i)
                    {
                        if (strcmp(struct_def->struct_members[i]->member_name, var->variable_struct_member->variable_name) == 0)
                        {
                            return def->variable_def_value->init_list_values[i];
                        }
                    }
                }

                return def;
            }
        }
    }

    for (size_t i = 0; i < scope->curr_layer->nparams; ++i)
    {
        struct Node *param = scope->curr_layer->params[i];

        if (strcmp(param->param_name, var->variable_name) == 0)
            return param;
    }

    return 0;
}


struct Node *scope_find_function(struct Scope *scope, char *name)
{
    for (size_t i = 0; i < scope->function_defs_size; ++i)
    {
        if (strcmp(scope->function_defs[i]->function_def_name, name) == 0)
            return scope->function_defs[i];
    }

    return 0;
}


struct Node *scope_find_struct(struct Scope *scope, char *name)
{
    for (size_t i = 0; i < scope->struct_defs_size; ++i)
    {
        if (strcmp(scope->struct_defs[i]->struct_name, name) == 0)
            return scope->struct_defs[i];
    }

    return 0;
}


void scope_pop_layer(struct Scope *scope)
{
    layer_free(scope->layers[scope->nlayers - 1]);
    scope->layers = realloc(scope->layers, sizeof(struct ScopeLayer*) * --scope->nlayers);

    scope->curr_layer = scope->layers[scope->nlayers - 1];
}


void scope_push_layer(struct Scope *scope)
{
    scope->layers = realloc(scope->layers, sizeof(struct ScopeLayer*) * ++scope->nlayers);
    scope->layers[scope->nlayers - 1] = layer_alloc();

    scope->curr_layer = scope->layers[scope->nlayers - 1];
}


void scope_combine(struct Scope *s1, struct Scope *s2)
{
    s1->function_defs = realloc(s1->function_defs,
        sizeof(struct Node*) * (s1->function_defs_size + s2->function_defs_size));

    for (size_t i = 0; i < s2->function_defs_size; ++i)
        s1->function_defs[s1->function_defs_size + i] = s2->function_defs[i];

    s1->function_defs_size += s2->function_defs_size;

    s1->struct_defs = realloc(s1->struct_defs,
        sizeof(struct Node*) * (s1->struct_defs_size + s2->struct_defs_size));

    for (size_t i = 0; i < s2->struct_defs_size; ++i)
        s1->struct_defs[s1->struct_defs_size + i] = s2->struct_defs[i];

    s1->struct_defs_size += s2->struct_defs_size;
}

