#include "asm.h"
#include "util.h"
#include "errors.h"
#include "crust.h"
#include "parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

#define MAX_INT_LEN 10

struct Asm *asm_alloc(struct Args *args, bool main)
{
    struct Asm *as = malloc(sizeof(struct Asm));

    const char *data_template = ".section .data\n";
    as->data = calloc(strlen(data_template) + 1, sizeof(char));
    strcpy(as->data, data_template);

    if (main)
    {
        const char *begin = ".globl _start\n"
                            "_start:\n"
                            "call main\n"
                            "mov $1, %eax\n"
                            "int $0x80\n";

        as->root = calloc(strlen(begin) + 1, sizeof(char));
        strcpy(as->root, begin);
    }
    else
    {
        as->root = calloc(1, sizeof(char));
    }

    as->scope = scope_alloc();
    // Global scope
    scope_push_layer(as->scope);

    as->args = args;

    return as;
}


void asm_free(struct Asm *as)
{
    free(as->data);
    free(as->root);
    scope_free(as->scope);
    free(as);
}


void asm_gen_expr(struct Asm *as, struct Node *node)
{
    switch (node->type)
    {
    case NODE_COMPOUND:
        for (size_t i = 0; i < node->compound_size; ++i)
            asm_gen_expr(as, node->compound_nodes[i]);
        break;
    case NODE_FUNCTION_DEF:
        if (node->function_def_is_decl)
            return;

        errors_asm_check_function_def(as->scope, node);
        scope_add_function_def(as->scope, node);
        asm_gen_function_def(as, node);
        break;
    case NODE_RETURN:
        asm_gen_return(as, node);
        break;
    case NODE_VARIABLE_DEF:
        scope_add_variable_def(as->scope, node);
        asm_gen_variable_def(as, node);
        break;
    case NODE_FUNCTION_CALL:
        asm_gen_function_call(as, node);
        break;
    case NODE_ASSIGNMENT:
        asm_gen_assignment(as, node);
        break;
    case NODE_STRUCT:
        scope_add_struct_def(as->scope, node);
        break;
    case NODE_INCLUDE:
        scope_combine(as->scope, node->include_scope);
        break;
    case NODE_BINOP:
        asm_gen_binop(as, node);
        break;
    default: break;
    }
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

    util_strcat(&as->root, s);
    free(s);

    scope_push_layer(as->scope);

    as->scope->curr_layer->params = node->function_def_params;
    as->scope->curr_layer->nparams = node->function_def_params_size;

    for (size_t i = 0; i < node->function_def_body->compound_size; ++i)
        asm_gen_expr(as, node->function_def_body->compound_nodes[i]);

    if (node->function_def_return_type.type == NODE_NOOP)
        util_strcat(&as->root, "movl $0, %ebx\nleave\nret\n");

    errors_asm_check_function_return(as->scope, node);

    if (as->args->warnings[WARNING_UNUSED_VARIABLE])
        errors_warn_unused_variable(as->scope, node);

    scope_pop_layer(as->scope);

    if (as->args->warnings[WARNING_DEAD_CODE])
        errors_warn_dead_code(node);
}


void asm_gen_return(struct Asm *as, struct Node *node)
{
    const char *template =  "movl %s, %%ebx\n"
                            "leave\n"
                            "ret\n";

    asm_gen_expr(as, node->return_value);
    char *ret = asm_str_from_node(as, node->return_value);

    size_t len = strlen(template) + strlen(ret);
    char *s = calloc(len + 1, sizeof(char));
    sprintf(s, template, ret);
    s = realloc(s, sizeof(char) * (strlen(s) + 1));

    util_strcat(&as->root, s);

    free(s);
    free(ret);
}


void asm_gen_variable_def(struct Asm *as, struct Node *node)
{
    struct Node *literal = node_strip_to_literal(node, as->scope);
    asm_gen_expr(as, node->variable_def_value);

    asm_gen_add_to_stack(as, literal, node->variable_def_stack_offset);
    errors_asm_check_variable_def(as->scope, node);
}


void asm_gen_add_to_stack(struct Asm *as, struct Node *node, int stack_offset)
{
    if (node->type == NODE_INIT_LIST)
    {
        errors_asm_check_init_list(as->scope, node);

        for (size_t i = 0; i < node->init_list_len; ++i)
            asm_gen_add_to_stack(as, node->init_list_values[i], stack_offset + 4 * i);

        return;
    }

    if (node->type == NODE_STRING)
        asm_gen_store_string(as, node);

    const char *template =  "subl $4, %%esp\n"
                            "movl %s, %d(%%ebp)\n";

    char *left = asm_str_from_node(as, node);

    size_t len = strlen(left) + MAX_INT_LEN + strlen(template);
    char *s = calloc(len + 1, sizeof(char));
    sprintf(s, template, left, stack_offset);
    s = realloc(s, sizeof(char) * (strlen(s) + 1));

    free(left);

    util_strcat(&as->root, s);
    free(s);
}


void asm_gen_store_string(struct Asm *as, struct Node *node)
{
    if (asm_check_lc_defined(as, node->string_asm_id))
        return;

    const char *template = "%s: .asciz \"%s\"\n";

    size_t len = strlen(template) + strlen(node->string_value) + MAX_INT_LEN;
    char *s = calloc(len + 1, sizeof(char));
    sprintf(s, template, node->string_asm_id, node->string_value);

    util_strcat(&as->data, s);

    free(s);
}


void asm_gen_function_call(struct Asm *as, struct Node *node)
{
    if (strcmp(node->function_call_name, "pront") == 0)
    {
        asm_gen_builtin_print(as, node);
        return;
    }

    struct Node *func = scope_find_function(as->scope, node->function_call_name);

    errors_asm_check_function_call(as->scope, func, node);

    // Push args on stack backwards so they're in order
    for (int i = node->function_call_args_size - 1; i >= 0; --i)
    {
        asm_gen_expr(as, node->function_call_args[i]);

        const char *template = "pushl %s\n";
        struct Node *arg = node_strip_to_literal(node->function_call_args[i], as->scope);
        char *value = asm_str_from_node(as, arg);

        size_t len = strlen(template) + strlen(value);
        char *s = calloc(len + 1, sizeof(char));
        sprintf(s, template, value);

        util_strcat(&as->root, s);
        free(s);
        free(value);
    }

    const char *template = "call %s\n"
                           "subl $4, %%esp\n"
                           "movl %%ebx, %d(%%ebp)\n";

    size_t len = strlen(template) + strlen(node->function_call_name) + MAX_INT_LEN;
    char *s = calloc(len + 1, sizeof(char));
    sprintf(s, template, node->function_call_name, node->function_call_return_stack_offset);
    s = realloc(s, sizeof(char) * (strlen(s) + 1));

    util_strcat(&as->root, s);
    free(s);
}


void asm_gen_assignment(struct Asm *as, struct Node *node)
{
    asm_gen_expr(as, node->assignment_src);
    errors_asm_check_assignment(as->scope, node);

    char *src = asm_str_from_node(as, node->assignment_src);
    char *dst = asm_str_from_node(as, node->assignment_dst);

    char *template;

    // Avoid too many memory references in one mov instruction
    if (isdigit(src[src[0] == '-' ? 1 : 0]) && isdigit(dst[dst[0] == '-' ? 1 : 0]))
        template =  "movl %s, %%ecx\n"
                    "movl %%ecx, %s\n";
    else
        template = "movl %s, %s\n";

    size_t len = strlen(template) + strlen(src) + strlen(dst);
    char *s = calloc(len + 1, sizeof(char));
    sprintf(s, template, src, dst);
    s = realloc(s, sizeof(char) * (strlen(s) + 1));

    util_strcat(&as->root, s);

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


void asm_gen_binop(struct Asm *as, struct Node *node)
{
    const char *p1 = "movl %s, %%eax\n";
    const char *p2 = "movl %s, %%ecx\n";

    char *left, *right;

    if (node->op_l->type == NODE_BINOP)
    {
        left = util_strcpy("%ecx");
    }
    else
    {
        left = asm_str_from_node(as, node->op_l);
    }

    right = asm_str_from_node(as, node->op_r);

    asm_gen_expr(as, node->op_l);
    char *s1 = calloc(strlen(p1) + strlen(left) + 1, sizeof(char));
    sprintf(s1, p1, left);
    util_strcat(&as->root, s1);
    free(s1);

    asm_gen_expr(as, node->op_r);
    char *s2 = calloc(strlen(p2) + strlen(right) + 1, sizeof(char));
    sprintf(s2, p2, right);
    util_strcat(&as->root, s2);
    free(s2);

    free(left);
    free(right);

    switch (node->op_type)
    {
    case OP_PLUS:
        util_strcat(&as->root, "addl %eax, %ecx\n"); break;
    case OP_MINUS:
        util_strcat(&as->root, "subl %ecx, %eax\n"); break;
    case OP_MUL:
        util_strcat(&as->root, "imull %eax, %ecx\n"); break;
    case OP_DIV:
        util_strcat(&as->root, "idivl %ecx\nmovl %eax, %ecx\n"); break;
    }
}


void asm_gen_builtin_print(struct Asm *as, struct Node *node)
{
    // TODO Replace with custom print written in asm
    const char *template =  "movl $%d, %%edx\n"
                            "movl $1, %%ebx\n"
                            "movl $4, %%eax\n"
                            "movl %s, %%ecx\n"
                            "int $0x80\n";

    for (size_t i = 0; i < node->function_call_args_size; ++i)
    {
        asm_gen_expr(as, node->function_call_args[i]);

        char *value = asm_str_from_node(as, node->function_call_args[i]);

        size_t len = strlen(template) + strlen(value) + MAX_INT_LEN;

        char *s = calloc(len + 1, sizeof(char));
        sprintf(s, template, 5, value);

        util_strcat(&as->root, s);
        free(s);
        free(value);
    }
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
    case NODE_BINOP: return asm_str_from_binop(as, node);
    default:
        errors_asm_str_from_node(node);
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
    struct Node *var = scope_find_variable(as->scope, node);

    if (!var)
        errors_asm_nonexistent_variable(node);

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
    // Redundant moving because gas complains about too many memory references
    const char *template = "movl %d(%%ebp), %%ecx\n";
    char *s = calloc(strlen(template) + MAX_INT_LEN + 1, sizeof(char));
    sprintf(s, template, node->function_call_return_stack_offset);
    util_strcat(&as->root, s);
    free(s);

    return util_strcpy("%ecx");
}


char *asm_str_from_binop(struct Asm *as, struct Node *node)
{
    return util_strcpy("%ecx");
}


bool asm_check_lc_defined(struct Asm *as, char *string_asm_id)
{
    for (int i = 0; i < strlen(as->data); ++i)
    {
        if (as->data[i] == '\n')
        {
            int prev = i;
            char buf[MAX_INT_LEN + 3] = { 0 };

            while (as->data[i] != '\0' && as->data[++i] != ':')
                buf[i - prev - 1] = as->data[i];

            if (strcmp(string_asm_id, buf) == 0)
                return true;
        }
    }

    return false;
}

