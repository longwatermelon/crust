#include "asm.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>


struct Asm *asm_alloc()
{
    struct Asm *as = malloc(sizeof(struct Asm));

    const char *data_template = ".section .data\n";
    as->data = calloc(strlen(data_template) + 1, sizeof(char));
    strcpy(as->data, data_template);

    const char *root_template = ".section .text\n"
                                ".globl _start\n"
                                "_start:\n"
                                "call main\n"
                                "mov $1, %eax\n"
                                "int $0x80\n";
    as->root = calloc(strlen(root_template) + 1, sizeof(char));
    strcpy(as->root, root_template);

    as->scope = scope_alloc();

    return as;
}


void asm_free(struct Asm *as)
{
    free(as->data);
    free(as->root);
    scope_free(as->scope);
    free(as);
}


char *asm_gen_root(struct Asm *as, struct Node *node)
{
    for (size_t i = 0; i < node->compound_size; ++i)
        asm_gen(as, node->compound_nodes[i]);

    char *str = malloc(sizeof(char) * (strlen(as->data) + strlen(as->root) + 1));
    sprintf(str, "%s%s", as->data, as->root);
    str[strlen(as->data) + strlen(as->root)] = '\0';

    return str;
}


void asm_gen(struct Asm *as, struct Node *node)
{
    if (node->type == NODE_FUNCTION_DEF)
        asm_gen_function_def(as, node);
    if (node->type == NODE_RETURN)
        asm_gen_return(as, node);
    if (node->type == NODE_VARIABLE_DEF)
    {
        scope_add_variable_def(as->scope, node);
        asm_gen_variable_def(as, node);
    }
    if (node->type == NODE_FUNCTION_CALL)
        asm_gen_function_call(as, node);
}


void asm_gen_function_def(struct Asm *as, struct Node *node)
{
    const char *template = ".globl %s\n"
                            "%s:\n";

    size_t len = strlen(template) + strlen(node->function_def_name) * 2;
    char *s = calloc(len + 1, sizeof(char));
    sprintf(s, template, node->function_def_name, node->function_def_name);
    s = realloc(s, sizeof(char) * (strlen(s) + 1));

    as->root = realloc(as->root, sizeof(char) * (strlen(as->root) + strlen(s) + 1));
    strcat(as->root, s);
    free(s);

    for (size_t i = 0; i < node->function_def_body->compound_size; ++i)
        asm_gen(as, node->function_def_body->compound_nodes[i]);
}


void asm_gen_return(struct Asm *as, struct Node *node)
{
    // %sbx -> temporary hack to get around % being accepted as a format specifier in sprintf
    const char *template = "mov $%s, %sbx\n"
                            "ret\n";

    char *ret = 0;

    switch (node->return_value->type)
    {
    case NODE_INT:
        ret = util_int_to_str(node->return_value->int_value);
    default:
        break;
    }

    if (!ret)
    {
        fprintf(stderr, "Returning invalid data of type %d\n", node->return_value->type);
        exit(EXIT_FAILURE);
    }

    size_t len = strlen(template) + strlen(ret);
    char *s = calloc(len + 1, sizeof(char));
    sprintf(s, template, ret, "%e");
    s = realloc(s, sizeof(char) * (strlen(s) + 1));

    as->root = realloc(as->root, sizeof(char) * (strlen(as->root) + strlen(s) + 1));
    strcat(as->root, s);

    free(s);
    free(ret);
}


void asm_gen_variable_def(struct Asm *as, struct Node *node)
{
    // TODO Use appropriate types for int; string representation is only for testing
    const char *template = "%s: %s \"%s\"\n";

    char *type = 0, *value = 0;

    switch (node->variable_def_type)
    {
    case NODE_INT:
        type = ".string";
        value = util_int_to_str(node->variable_def_value->int_value);
        break;
    default: break;
    }

    if (!type)
    {
        fprintf(stderr, "Assigning invalid data of type %d to variable %s\n",
                        node->variable_def_type, node->variable_def_name);
        exit(EXIT_FAILURE);
    }

    size_t len = strlen(template) + strlen(node->variable_def_name);
    char *s = calloc(len + 1, sizeof(char));

    sprintf(s, template, node->variable_def_name, type, value);
    s = realloc(s, sizeof(char) * (strlen(s) + 1));

    as->data = realloc(as->data, sizeof(char) * (strlen(as->data) + strlen(s) + 1));
    strcat(as->data, s);

    free(value);
    free(s);
}


void asm_gen_variable(struct Asm *as, struct Node *node)
{
    struct Node *var = scope_find_variable(as->scope, node->variable_name);

    if (!var)
    {
        fprintf(stderr, "No variable named '%s' defined\n", node->variable_name);
        exit(EXIT_FAILURE);
    }

    switch (var->variable_def_type)
    {
    case NODE_INT:
    {
        char *s = util_int_to_str(var->variable_def_value->int_value);
        as->root = realloc(as->root, sizeof(char) * (strlen(as->root) + strlen(s) + 1));
        strcat(as->root, s);
        free(s);
    } break;
    default: break;
    }
}


void asm_gen_function_call(struct Asm *as, struct Node *node)
{
    if (strcmp(node->function_call_name, "pront") == 0)
        return asm_gen_builtin_print(as, node);

    // TODO Call user defined functions
    fprintf(stderr, "Unrecognized function '%s'\n", node->function_call_name);
    exit(EXIT_FAILURE);
}


void asm_gen_builtin_print(struct Asm *as, struct Node *node)
{
    const char *template = "mov $%d, %sdx\n" // %edx
                            "mov $%s, %scx\n" // %ecx
                            "mov $1, %sbx\n" // %ebx
                            "mov $4, %sax\n" // %eax
                            "int $0x80\n";

    struct Node *var = scope_find_variable(as->scope, node->function_call_args[0]->variable_name);

    int edx_len = 0;
    switch (var->variable_def_type)
    {
    case NODE_INT:
    {
        char *tmp = util_int_to_str(var->variable_def_value->int_value);
        edx_len = strlen(tmp);
        free(tmp);
    } break;
    default: break;
    }

    size_t len = strlen(template) + strlen(node->function_call_args[0]->variable_name) + edx_len;
    char *s = calloc(len + 1, sizeof(char));
    sprintf(s, template, edx_len, "%e", node->function_call_args[0]->variable_name, "%e", "%e", "%e");

    as->root = realloc(as->root, sizeof(char) * (strlen(as->root) + strlen(s) + 1));
    strcat(as->root, s);
}

