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

    const char *begin = ".section .text\n"
                        ".globl _start\n"
                        "_start:\n"
                        "call main\n"
                        "mov $1, %eax\n"
                        "int $0x80\n";

    as->root = calloc(strlen(begin) + 1, sizeof(char));
    strcpy(as->root, begin);

    as->scope = scope_alloc();
    as->lc = 0;
    as->stack_size = 0;

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
        asm_gen_expr(as, node->compound_nodes[i]);

    size_t len = strlen(as->data) + strlen(as->root);
    char *str = calloc(len + 1, sizeof(char));
    sprintf(str, "%s%s", as->data, as->root);
    str = realloc(str, sizeof(char) * (strlen(str) + 1));

    return str;
}


void asm_gen_expr(struct Asm *as, struct Node *node)
{
    if (node->type == NODE_FUNCTION_DEF)
    {
        scope_add_function_def(as->scope, node);
        asm_gen_function_def(as, node);
    }

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
    const char *template =  ".globl %s\n"
                            "%s:\n"
                            "pushl %sbp\n" // %ebp
                            "movl %ssp, %sbp\n"; // %esp, %ebp

    size_t len = strlen(template) + strlen(node->function_def_name) * 2;
    char *s = calloc(len + 1, sizeof(char));
    sprintf(s, template, node->function_def_name, node->function_def_name, "%e", "%e", "%e");
    s = realloc(s, sizeof(char) * (strlen(s) + 1));

    asm_append_str(&as->root, s);
    free(s);

    size_t prev_size = as->stack_size;
    struct Node **variable_defs = as->scope->variable_defs;
    size_t variable_defs_size = as->scope->variable_defs_size;
    as->scope->variable_defs = 0;
    as->scope->variable_defs_size = 0;

    for (size_t i = 0; i < node->function_def_body->compound_size; ++i)
        asm_gen_expr(as, node->function_def_body->compound_nodes[i]);

    as->stack_size = prev_size;
    as->scope->variable_defs = variable_defs;
    as->scope->variable_defs_size = variable_defs_size;
}


void asm_gen_return(struct Asm *as, struct Node *node)
{
    // %sbx -> temporary hack to get around % being accepted as a format specifier in sprintf
    const char *template =  "movl $%s, %sbx\n"
                            "leave\n"
                            "ret\n";

    char *ret = 0;

    switch (node->return_value->type)
    {
    case NODE_INT:
        ret = util_int_to_str(node->return_value->int_value);
        break;
    case NODE_STRING:
        asm_gen_store_string(as, node->return_value);
        ret = malloc(sizeof(char) * (strlen(node->return_value->string_asm_id) + 1));
        strcpy(ret, node->return_value->string_asm_id);
        break;
    default:
        fprintf(stderr, "Returning invalid data of type %d\n", node->return_value->type);
        exit(EXIT_FAILURE);
        break;
    }

    size_t len = strlen(template) + strlen(ret);
    char *s = calloc(len + 1, sizeof(char));
    sprintf(s, template, ret, "%e");
    s = realloc(s, sizeof(char) * (strlen(s) + 1));

    asm_append_str(&as->root, s);

    free(s);
    free(ret);
}


void asm_gen_variable_def(struct Asm *as, struct Node *node)
{
    struct Node *literal = asm_eval_node(as, node);

    // Creating a new label is only necessary for strings
    if (node->variable_def_type == NODE_STRING)
        asm_gen_store_string(as, literal);

    asm_gen_add_to_stack(as, literal);
}


void asm_gen_add_to_stack(struct Asm *as, struct Node *node)
{
    as->stack_size += 4;
    const char *template =  "subl $4, %ssp\n" // %esp
                            "movl $%s, -%s(%sbp)\n"; // %ebp

    char *left = 0;
    char *stack_size_str = util_int_to_str(as->stack_size);

    switch (node->type)
    {
    case NODE_INT: left = util_int_to_str(node->int_value); break;
    case NODE_STRING:
        left = malloc(sizeof(char) * (strlen(node->string_asm_id) + 1));
        strcpy(left, node->string_asm_id);
        break;
    default:
        fprintf(stderr, "Can't add data of type %d to stack\n", node->type);
        exit(EXIT_FAILURE);
    }

    size_t len = strlen(left) + strlen(stack_size_str) + strlen(template);
    char *s = calloc(len + 1, sizeof(char));
    sprintf(s, template, "%e", left, stack_size_str, "%e");
    s = realloc(s, sizeof(char) * (strlen(s) + 1));

    free(left);
    free(stack_size_str);

    asm_append_str(&as->root, s);
    free(s);
}


void asm_gen_store_string(struct Asm *as, struct Node *node)
{
    node = asm_eval_node(as, node);
    const char *template = ".LC%s: .string \"%s\"\n";

    char *lc = util_int_to_str(as->lc);
    size_t len = strlen(template) + strlen(node->string_value) + strlen(lc);
    char *s = calloc(len + 1, sizeof(char));
    sprintf(s, template, lc, node->string_value);

    asm_append_str(&as->data, s);

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

    struct Node *func = scope_find_function(as->scope, node->function_call_name);

    if (func->function_def_params_size != node->function_call_args_size)
    {
        fprintf(stderr, "Argument number mistmatch; function '%s'"
                        "has %lu parameters but %lu arguments were provided\n",
                        func->function_def_name, func->function_def_params_size,
                        node->function_call_args_size);
        exit(EXIT_FAILURE);
    }

    const char *template = "call %s\n";
    size_t len = strlen(template) + strlen(node->function_call_name);
    char *s = calloc(len + 1, sizeof(char));
    sprintf(s, template, node->function_call_name);
    s = realloc(s, sizeof(char) * (strlen(s) + 1));

    asm_append_str(&as->root, s);
    free(s);
}


void asm_gen_builtin_print(struct Asm *as, struct Node *node)
{
    const char *template =  "movl $%d, %sdx\n" // %edx
                            "movl $%s, %scx\n" // %ecx
                            "movl $1, %sbx\n" // %ebx
                            "movl $4, %sax\n" // %eax
                            "int $0x80\n";

    for (size_t i = 0; i < node->function_call_args_size; ++i)
    {
        struct Node *arg = asm_eval_node(as, node->function_call_args[i]);
        struct Node *value = 0;

        if (arg->type == NODE_STRING)
        {
            value = arg;
            asm_gen_store_string(as, value);
        }
        else
        {
            fprintf(stderr, "Unable to print data of type %d\n", arg->type);
            exit(EXIT_FAILURE);
        }

        size_t len = strlen(template) + strlen(value->string_asm_id)
                    + strlen(value->string_value);

        char *s = calloc(len + 1, sizeof(char));
        sprintf(s, template, strlen(value->string_value), "%e",
                        value->string_asm_id, "%e", "%e", "%e");

        asm_append_str(&as->root, s);
        free(s);
    }
}


struct Node *asm_eval_node(struct Asm *as, struct Node *node)
{
    switch (node->type)
    {
    case NODE_INT:
    case NODE_STRING:
        return node;
    case NODE_VARIABLE:
        return asm_eval_node(as, scope_find_variable(as->scope, node->variable_name));
    case NODE_VARIABLE_DEF:
        return asm_eval_node(as, node->variable_def_value);
    default: return 0;
    }
}


void asm_append_str(char **dst, char *src)
{
    *dst = realloc(*dst, sizeof(char) * (strlen(*dst) + strlen(src) + 1));
    strcat(*dst, src);
}

