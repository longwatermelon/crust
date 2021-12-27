#include "scope.h"
#include <string.h>
#include <stdio.h>


struct Scope *scope_alloc()
{
    struct Scope *scope = malloc(sizeof(struct Scope));
    scope->variable_defs = 0;
    scope->variable_defs_size = 0;

    return scope;
}


void scope_free(struct Scope *scope)
{
    if (scope->variable_defs)
        free(scope->variable_defs);

    free(scope);
}


void scope_add_variable_def(struct Scope *scope, struct Node *node)
{
    scope->variable_defs = realloc(scope->variable_defs, sizeof(struct Node*) * ++scope->variable_defs_size);
    scope->variable_defs[scope->variable_defs_size - 1] = node;
}


void scope_add_function_def(struct Scope *scope, struct Node *node)
{
    scope->function_defs = realloc(scope->function_defs, sizeof(struct Node*) * ++scope->function_defs_size);
    scope->function_defs[scope->function_defs_size - 1] = node;
}



struct Node *scope_find_variable(struct Scope *scope, char *name)
{
    for (size_t i = 0; i < scope->variable_defs_size; ++i)
    {
        if (strcmp(scope->variable_defs[i]->variable_def_name, name) == 0)
            return scope->variable_defs[i];
    }

    fprintf(stderr, "No variable named '%s'\n", name);
    exit(EXIT_FAILURE);
}


struct Node *scope_find_function(struct Scope *scope, char *name)
{
    for (size_t i = 0; i < scope->function_defs_size; ++i)
    {
        if (strcmp(scope->function_defs[i]->function_def_name, name) == 0)
            return scope->function_defs[i];
    }

    fprintf(stderr, "No function named '%s'\n", name);
    exit(EXIT_FAILURE);
}

