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
    // Global scope even though it holds nothing
    scope_push_layer(as->scope);

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
                            "pushl %%ebp\n"
                            "movl %%esp, %%ebp\n";

    size_t len = strlen(template) + strlen(node->function_def_name) * 2;
    char *s = calloc(len + 1, sizeof(char));
    sprintf(s, template, node->function_def_name, node->function_def_name);
    s = realloc(s, sizeof(char) * (strlen(s) + 1));

    asm_append_str(&as->root, s);
    free(s);

    size_t prev_size = as->stack_size;
    scope_push_layer(as->scope);

    as->scope->curr_layer->params = node->function_def_params;
    as->scope->curr_layer->nparams = node->function_def_params_size;

    for (size_t i = 0; i < node->function_def_body->compound_size; ++i)
        asm_gen_expr(as, node->function_def_body->compound_nodes[i]);

    as->stack_size = prev_size;
    scope_pop_layer(as->scope);
}


void asm_gen_return(struct Asm *as, struct Node *node)
{
    const char *template =  "movl %s, %%ebx\n"
                            "leave\n"
                            "ret\n";

    char *ret = asm_str_from_node(as, node->return_value);

    size_t len = strlen(template) + strlen(ret);
    char *s = calloc(len + 1, sizeof(char));
    sprintf(s, template, ret);
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
    const char *template =  "subl $4, %%esp\n"
                            "movl $%s, -%s(%%ebp)\n";

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
    sprintf(s, template, left, stack_size_str);
    s = realloc(s, sizeof(char) * (strlen(s) + 1));

    free(left);
    free(stack_size_str);

    asm_append_str(&as->root, s);
    free(s);
}


void asm_gen_store_string(struct Asm *as, struct Node *node)
{
    node = asm_eval_node(as, node);

    if (node->string_asm_id)
        return;

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

    for (size_t i = 0; i < node->function_call_args_size; ++i)
    {
        const char *template = "pushl %s\n";
        struct Node *arg = asm_eval_node(as, node->function_call_args[i]);
        char *value = asm_str_from_node(as, arg);

        size_t len = strlen(template) + strlen(value);
        char *s = calloc(len + 1, sizeof(char));
        sprintf(s, template, value);

        asm_append_str(&as->root, s);
        free(s);
        free(value);
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
    // TODO Replace with custom print written in asm
    const char *template =  "movl $%d, %%edx\n"
                            "movl %s, %%ecx\n"
                            "movl $1, %%ebx\n"
                            "movl $4, %%eax\n"
                            "int $0x80\n";

    for (size_t i = 0; i < node->function_call_args_size; ++i)
    {
        struct Node *arg = asm_eval_node(as, node->function_call_args[i]);
        char *value = asm_str_from_node(as, arg);

        size_t len = strlen(template) + strlen(value) + 12;

        char *s = calloc(len + 1, sizeof(char));
        sprintf(s, template, 5, value);

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
    default: return node;
    }
}


void asm_append_str(char **dst, char *src)
{
    *dst = realloc(*dst, sizeof(char) * (strlen(*dst) + strlen(src) + 1));
    strcat(*dst, src);
}


char *asm_str_from_node(struct Asm *as, struct Node *node)
{
    switch (node->type)
    {
    case NODE_INT: return asm_str_from_int(as, node);
    case NODE_STRING: return asm_str_from_str(as, node);
    case NODE_VARIABLE: return asm_str_from_var(as, node);
    case NODE_PARAMETER: return asm_str_from_param(as, node);
    default:
        fprintf(stderr, "Cannot extract value from data of type %d\n", node->type);
        exit(EXIT_FAILURE);
        break;
    }

    return 0;
}


char *asm_str_from_int(struct Asm *as, struct Node *node)
{
    char *num = util_int_to_str(node->int_value);
    char *value = malloc(sizeof(char) * (strlen(num) + 2));
    value[0] = '$';
    strcpy(&value[1], num);
    free(num);
    return value;
}


char *asm_str_from_str(struct Asm *as, struct Node *node)
{
    asm_gen_store_string(as, node);
    char *value = malloc(sizeof(char) * (strlen(node->string_asm_id) + 2));
    value[0] = '$';
    strcpy(&value[1], node->string_asm_id);
    return value;
}


char *asm_str_from_var(struct Asm *as, struct Node *node)
{
    struct Node *var = scope_find_variable(as->scope, node->variable_name);

    if (var->type == NODE_PARAMETER)
    {
        return asm_str_from_param(as, var);
    }
    else
    {
        struct Node *literal = asm_eval_node(as, var);
        return asm_str_from_node(as, literal);
    }
}


char *asm_str_from_param(struct Asm *as, struct Node *node)
{
    const char *tmp = "%d(%%ebp)";

    char *value = calloc(strlen(tmp) + 12, sizeof(char));
    sprintf(value, tmp, node->param_stack_offset);
    value = realloc(value, sizeof(char) * (strlen(value) + 1));
    return value;
}

