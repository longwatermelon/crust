#include "asm.h"
#include "util.h"
#include "errors.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

#define MAX_INT_LEN 10

struct Asm *asm_alloc(const char *fp)
{
    struct Asm *as = malloc(sizeof(struct Asm));

    const char *data_template = ".section .data\n";
    as->data = calloc(strlen(data_template) + 1, sizeof(char));
    strcpy(as->data, data_template);

    const char *begin = ".globl _start\n"
                        "_start:\n"
                        "call main\n"
                        "mov $1, %eax\n"
                        "int $0x80\n";

    as->root = calloc(strlen(begin) + 1, sizeof(char));
    strcpy(as->root, begin);

    as->scope = scope_alloc();
    // Global scope
    scope_push_layer(as->scope);

    as->lc = 0;
    as->stack_size = 0;

    as->source = util_read_file_lines(fp, &as->source_size);

    return as;
}


void asm_free(struct Asm *as)
{
    for (size_t i = 0; i < as->source_size; ++i)
        free(as->source[i]);

    free(as->source);

    free(as->data);
    free(as->root);
    scope_free(as->scope);
    free(as);
}


void asm_gen_expr(struct Asm *as, struct Node *node)
{
    if (node->type == NODE_COMPOUND)
    {
        for (size_t i = 0; i < node->compound_size; ++i)
            asm_gen_expr(as, node->compound_nodes[i]);
    }

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

    if (node->type == NODE_ASSIGNMENT)
        asm_gen_assignment(as, node);
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
    as->stack_size = 0;
    scope_push_layer(as->scope);

    as->scope->curr_layer->params = node->function_def_params;
    as->scope->curr_layer->nparams = node->function_def_params_size;

    for (size_t i = 0; i < node->function_def_body->compound_size; ++i)
        asm_gen_expr(as, node->function_def_body->compound_nodes[i]);

    errors_check_function_return(node, as);

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
    struct Node *literal = node_strip_to_literal(node, as->scope);

    // Creating a new label is only necessary for strings
    if (literal->type == NODE_STRING)
        asm_gen_store_string(as, literal);

    asm_gen_add_to_stack(as, literal);
    node->variable_def_stack_offset = -as->stack_size;

    errors_check_variable_def(node, as);
}


void asm_gen_add_to_stack(struct Asm *as, struct Node *node)
{
    as->stack_size += 4;
    const char *template =  "subl $4, %%esp\n"
                            "movl %s, -%d(%%ebp)\n";

    char *left = asm_str_from_node(as, node);

    size_t len = strlen(left) + MAX_INT_LEN + strlen(template);
    char *s = calloc(len + 1, sizeof(char));
    sprintf(s, template, left, as->stack_size);
    s = realloc(s, sizeof(char) * (strlen(s) + 1));

    free(left);

    asm_append_str(&as->root, s);
    free(s);
}


void asm_gen_store_string(struct Asm *as, struct Node *node)
{
    node = node_strip_to_literal(node, as->scope);

    if (node->string_asm_id)
        return;

    const char *template = ".LC%d: .asciz \"%s\"\n";

    size_t len = strlen(template) + strlen(node->string_value) + MAX_INT_LEN;
    char *s = calloc(len + 1, sizeof(char));
    sprintf(s, template, as->lc, node->string_value);

    asm_append_str(&as->data, s);

    size_t id_len = strlen(".LC") + MAX_INT_LEN;
    node->string_asm_id = calloc(id_len + 1, sizeof(char));
    sprintf(node->string_asm_id, ".LC%lu", as->lc);
    node->string_asm_id = realloc(node->string_asm_id, sizeof(char) * (strlen(node->string_asm_id) + 1));

    free(s);

    ++as->lc;
}


void asm_gen_function_call(struct Asm *as, struct Node *node)
{
    if (strcmp(node->function_call_name, "pront") == 0)
        return asm_gen_builtin_print(as, node);

    struct Node *func = scope_find_function(as->scope, node->function_call_name);

    errors_check_function_call(func, node, as);

    // Push args on stack backwards so they're in order
    for (int i = node->function_call_args_size - 1; i >= 0; --i)
    {
        const char *template = "pushl %s\n";
        struct Node *arg = node_strip_to_literal(node->function_call_args[i], as->scope);
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


void asm_gen_assignment(struct Asm *as, struct Node *node)
{
    errors_check_assignment(node, as);

    char *src = asm_str_from_node(as, node->assignment_src);
    char *dst = asm_str_from_node(as, node->assignment_dst);

    char *template;

     // Gas complains about code that looks like movl 4(%ebp), 8(%ebp)
    if (isdigit(src[src[0] == '-' ? 1 : 0]) && isdigit(dst[dst[0] == '-' ? 1 : 0]))
        template =  "movl %s, %%ecx\n"
                    "movl %%ecx, %s\n";
    else
        template = "movl %s, %s\n";

    size_t len = strlen(template) + strlen(src) + strlen(dst);
    char *s = calloc(len + 1, sizeof(char));
    sprintf(s, template, src, dst);
    s = realloc(s, sizeof(char) * (strlen(s) + 1));

    asm_append_str(&as->root, s);

    free(src);
    free(dst);
    free(s);

    struct Node *node_src = node_strip_to_literal(node->assignment_src, as->scope);
    struct Node *node_dst = node_strip_to_literal(node->assignment_dst, as->scope);

    if (node_dst->type == NODE_STRING && node_src->type == NODE_STRING)
    {
        free(node_dst->string_asm_id);
        node_dst->string_asm_id = malloc(sizeof(char) * (strlen(node_src->string_asm_id) + 1));
        strcpy(node_dst->string_asm_id, node_src->string_asm_id);
    }
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
        char *value = asm_str_from_node(as, node->function_call_args[i]);

        size_t len = strlen(template) + strlen(value) + MAX_INT_LEN;

        char *s = calloc(len + 1, sizeof(char));
        sprintf(s, template, 5, value);

        asm_append_str(&as->root, s);
        free(s);
        free(value);
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
    case NODE_FUNCTION_CALL: return asm_str_from_function_call(as, node);
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
    else if (var->type == NODE_VARIABLE_DEF)
    {
        const char *tmp = "%d(%%ebp)";
        char *s = calloc(strlen(tmp) + MAX_INT_LEN + 1, sizeof(char));
        sprintf(s, tmp, var->variable_def_stack_offset);
        s = realloc(s, sizeof(char) * (strlen(s) + 1));
        return s;
    }
    else
    {
        struct Node *literal = node_strip_to_literal(var, as->scope);
        return asm_str_from_node(as, literal);
    }
}


char *asm_str_from_param(struct Asm *as, struct Node *node)
{
    const char *tmp = "%d(%%ebp)";

    char *value = calloc(strlen(tmp) + MAX_INT_LEN + 1, sizeof(char));
    sprintf(value, tmp, node->param_stack_offset);
    value = realloc(value, sizeof(char) * (strlen(value) + 1));
    return value;
}


char *asm_str_from_function_call(struct Asm *as, struct Node *node)
{
    asm_gen_function_call(as, node);
    const char *tmp = "%ebx";
    char *s = malloc(sizeof(char) * (strlen(tmp) + 1));
    strcpy(s, tmp);
    return s;
}

