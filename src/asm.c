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
    as->lc = 0;

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
    // Only store data if string, other data types can be directly accessed
    if (node->variable_def_type == NODE_STRING)
        asm_gen_store_string(as, node->variable_def_value);
}


void asm_gen_store_string(struct Asm *as, struct Node *node)
{
    const char *template = ".LC%s: .string \"%s\"\n";

    char *lc = util_int_to_str(as->lc);
    size_t len = strlen(template) + strlen(node->string_value) + strlen(lc);
    char *s = calloc(len + 1, sizeof(char));
    sprintf(s, template, lc, node->string_value);

    as->data = realloc(as->data, sizeof(char) * (strlen(as->data) + strlen(s)));
    strcat(as->data, s);

    size_t id_len = strlen(".LC") + strlen(lc);
    node->string_asm_id = malloc(sizeof(char) * (id_len + 1));
    sprintf(node->string_asm_id, ".LC%lu", as->lc);
    node->string_asm_id[id_len] = '\0';

    free(s);
    free(lc);

    ++as->lc;
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

    for (size_t i = 0; i < node->function_call_args_size; ++i)
    {
        struct Node *arg = node->function_call_args[i];
        struct Node *value = 0;

        if (arg->type == NODE_VARIABLE)
            value = scope_find_variable(as->scope, arg->variable_name)->variable_def_value;
        else if (arg->type == NODE_STRING)
        {
            value = arg;
            asm_gen_store_string(as, value);
        }

        if (!value)
        {
            fprintf(stderr, "Unable to print data of type %d\n", arg->type);
            exit(EXIT_FAILURE);
        }

        size_t len = strlen(template) + strlen(value->string_asm_id)
                    + strlen(value->string_value);

        char *s = calloc(len + 1, sizeof(char));
        sprintf(s, template, strlen(value->string_value), "%e",
                        value->string_asm_id, "%e", "%e", "%e");

        as->root = realloc(as->root, sizeof(char) * (strlen(as->root) + strlen(s) + 1));
        strcat(as->root, s);
        free(s);
    }
}

