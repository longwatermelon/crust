#include "asm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>


char *asm_gen_root(struct Node *node)
{
    const char *root = ".section .data\n"
                        ".section .text\n"
                        ".globl _start\n"
                        "_start:\n"
                        "mov $1, %eax\n"
                        "call main\n"
                        "int $0x80\n";

    char *str = malloc(sizeof(char) * (strlen(root) + 1));
    strcpy(str, root);

    for (size_t i = 0; i < node->compound_size; ++i)
    {
        char *new = asm_gen(node->compound_nodes[i]);

        if (new)
        {
            str = realloc(str, sizeof(char) * (strlen(str) + strlen(new) + 1));
            strcat(str, new);
        }
    }

    return str;
}


char *asm_gen(struct Node *node)
{
    if (node->type == NODE_FUNCTION_DEF)
        return asm_gen_function_def(node);
    if (node->type == NODE_RETURN)
        return asm_gen_return(node);

    return 0;
}


char *asm_gen_function_def(struct Node *node)
{
    const char *template = ".globl %s\n"
                            "%s:\n";

    size_t len = strlen(template) + strlen(node->function_def_name) * 2;
    char *s = calloc(len + 1, sizeof(char));
    sprintf(s, template, node->function_def_name, node->function_def_name);
    s = realloc(s, sizeof(char) * (strlen(s) + 1));

    for (size_t i = 0; i < node->function_def_body->compound_size; ++i)
    {
        char *new = asm_gen(node->function_def_body->compound_nodes[i]);
        s = realloc(s, sizeof(char) * (strlen(s) + strlen(new) + 1));
        strcat(s, new);
    }

    return s;
}


char *asm_gen_return(struct Node *node)
{
    // %sbx -> temporary hack to get around % being accepted as a format specifier in sprintf
    const char *template = "mov $%s, %sbx\n"
                            "ret\n";

    char *ret = 0;

    switch (node->return_value->type)
    {
    case NODE_INT:
    {
        ret = malloc(sizeof(char) * ((int)((ceil(log10(node->return_value->int_value)) + 1) * sizeof(char)) + 1));
        sprintf(ret, "%d", node->return_value->int_value);
    } break;
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

    return s;
}

