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
#define MEMORY_REF(x) (isdigit(x[0]) || x[0] == '-')

struct Asm *asm_alloc(struct Args *args, bool main)
{
    struct Asm *as = malloc(sizeof(struct Asm));

    const char *data_template = ".section .data\n";
    as->data = calloc(strlen(data_template) + 1, sizeof(char));
    strcpy(as->data, data_template);

    as->root = calloc(1, sizeof(char));
    util_strcat(&as->root, ".section .text\n");

    if (main)
    {
        const char *begin = ".globl _start\n"
                            "_start:\n"
                            "call main\n"
                            "movl $1, %eax\n"
                            "int $0x80\n";

        util_strcat(&as->root, begin);
    }

    as->scope = scope_alloc();
    // Global scope
    scope_push_layer(as->scope);

    as->args = args;

    as->func_label = 1;

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
        {
            scope_add_function_def(as->scope, node);
            return;
        }

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

    case NODE_IDOF:
        asm_gen_expr(as, node->idof_original_expr);
        asm_gen_expr(as, node->idof_new_expr);
        break;

    case NODE_STRING:
        asm_gen_store_string(as, node);
        break;

    case NODE_INLINE_ASM:
        asm_gen_inline_asm(as, node);
        break;

    case NODE_IF:
        asm_gen_if_statement(as, node);
        break;

    default: break;
    }
}


void asm_gen_function_def(struct Asm *as, struct Node *node)
{
    const char *template =  "# Function def\n"
                            ".globl %s\n"
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

    asm_gen_expr(as, node->function_def_body);

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
    const char *template =  "# Return\n"
                            "movl %s, %%ebx\n"
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
            asm_gen_add_to_stack(as, node->init_list_values[i], stack_offset - 4 * i);

        return;
    }

    asm_gen_expr(as, node);

    const char *template =  "# Add value to stack\n"
                            "subl $4, %%esp\n"
                            "movl %s, %d(%%ebp)\n";

    char *left = asm_str_from_node(as, node);

    if (MEMORY_REF(left))
    {
        const char *tmp = "# Avoid too many memory references\n"
                          "movl %s, %%eax\n";
        char *s = calloc(strlen(template) + strlen(left) + 1, sizeof(char));
        sprintf(s, tmp, left);
        util_strcat(&as->root, s);

        free(left);
        free(s);

        left = util_strcpy("%eax");
    }

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

    size_t len = strlen(template) + strlen(node->string_value) + MAX_INT_LEN - 1;
    char *s = calloc(len + 1, sizeof(char));
    // &node->string_asm_id[1]: exclude the '$'
    sprintf(s, template, &node->string_asm_id[1], node->string_value);

    util_strcat(&as->data, s);

    free(s);
}


void asm_gen_function_call(struct Asm *as, struct Node *node)
{
    struct Node *func = scope_find_function(as->scope, node->function_call_name, node->error_line);

    errors_asm_check_function_call(as->scope, func, node);

    asm_gen_push_args(as, node);

    const char *template = "# Function call\n"
                           "call %s\n"
                           "subl $4, %%esp\n"
                           "movl %%ebx, %d(%%ebp)\n";

    size_t len = strlen(template) + strlen(node->function_call_name) + MAX_INT_LEN;
    char *s = calloc(len + 1, sizeof(char));
    sprintf(s, template, node->function_call_name, node->function_call_return_stack_offset);
    s = realloc(s, sizeof(char) * (strlen(s) + 1));

    util_strcat(&as->root, s);
    free(s);
}


void asm_gen_push_args(struct Asm *as, struct Node *node)
{
    util_strcat(&as->root, "# Push function call args\n");

    // Push args on stack backwards so they're in order
    for (int i = node->function_call_args_size - 1; i >= 0; --i)
    {
        asm_gen_expr(as, node->function_call_args[i]);
        NodeDType type = node_type_from_node(node->function_call_args[i], as->scope);

        if (type.type == NODE_STRUCT)
            asm_gen_push_args_struct(as, node->function_call_args[i]);
        else
            asm_gen_push_args_primitive(as, node->function_call_args[i]);
    }
}


void asm_gen_push_args_primitive(struct Asm *as, struct Node *node)
{
    const char *template = "pushl %s\n";
    struct Node *arg = node_strip_to_literal(node, as->scope);
    char *value = asm_str_from_node(as, arg);

    size_t len = strlen(template) + strlen(value);
    char *s = calloc(len + 1, sizeof(char));
    sprintf(s, template, value);

    util_strcat(&as->root, s);
    free(s);
    free(value);
}


void asm_gen_push_args_struct(struct Asm *as, struct Node *node)
{
    struct Node *list = node_strip_to_literal(node, as->scope);
    char *value = asm_str_from_node(as, list);

    const char *template = "# Push init list ptr into function call args\n"
                           "leal %s, %%eax\n"
                           "pushl %%eax\n";
    char *s = calloc(strlen(template) + strlen(value) + 1, sizeof(char));
    sprintf(s, template, value);

    util_strcat(&as->root, s);

    free(value);
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
        template =  "# Assignment: Avoid too many memory references\n"
                    "movl %s, %%ecx\n"
                    "movl %%ecx, %s\n";
    else
        template =  "# Assignment\n"
                    "movl %s, %s\n";

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
    util_strcat(&as->root, "# Binop left\n");
    asm_gen_expr(as, node->op_l);
    asm_gen_add_to_stack(as, node->op_l, node->op_stack_offset);

    util_strcat(&as->root, "# Binop right\n");
    asm_gen_expr(as, node->op_r);
    asm_gen_add_to_stack(as, node->op_r, node->op_stack_offset - 4);

    const char *prepare_registers = "# Prepare registers for math\n"
                                    "movl %d(%%ebp), %%eax\n"
                                    "movl %d(%%ebp), %%ecx\n";
    char *s = calloc(strlen(prepare_registers) + MAX_INT_LEN * 2 + 1, sizeof(char));
    sprintf(s, prepare_registers, node->op_stack_offset, node->op_stack_offset - 4);
    util_strcat(&as->root, s);
    free(s);

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
    case OP_CMP:
        asm_gen_binop_cmp(as, node);
    }
}


void asm_gen_binop_cmp(struct Asm *as, struct Node *node)
{
    const char *tmp = "cmpl %%eax, %%ecx\n"
                      "jne .L%zu\n" // label
                      "movl $1, %%ecx\n"
                      "jmp .L%zu\n" // label + 1
                      ".L%zu:\n" // label
                      "movl $0, %%ecx\n"
                      ".L%zu:\n"; // label + 1

    size_t len = strlen(tmp) + MAX_INT_LEN * 4;
    char *s = calloc(len + 1, sizeof(char));
    sprintf(s, tmp, as->func_label, as->func_label + 1, as->func_label, as->func_label + 1);
    util_strcat(&as->root, s);
    free(s);

    as->func_label += 3;
}


void asm_gen_inline_asm(struct Asm *as, struct Node *node)
{
    char *s = calloc(1, sizeof(char));

    for (size_t i = 0; i < node->asm_nargs; ++i)
    {
        asm_gen_expr(as, node->asm_args[i]);
        struct Node *literal = node_strip_to_literal(node->asm_args[i], as->scope);

        char *tmp;

        if (literal->type == NODE_STRING)
            tmp = literal->string_value;
        else
            tmp = asm_str_from_node(as, literal);

        util_strcat(&s, tmp);
    }

    util_strcat(&as->root, s);
    free(s);

    util_strcat(&as->root, "\n");
}


void asm_gen_if_statement(struct Asm *as, struct Node *node)
{
    asm_gen_expr(as, node->if_cond);
    char *s = asm_str_from_node(as, node->if_cond);

    const char *label_template = ".L%zu";
    char *label = calloc(strlen(label_template) + MAX_INT_LEN + 1, sizeof(char));
    sprintf(label, label_template, as->func_label);
    ++as->func_label;

    const char *tmp = "cmpl $0, %s\n"
                      "je %s\n";
    char *str = calloc(strlen(tmp) + strlen(s) + strlen(label) + 1, sizeof(char));
    sprintf(str, tmp, s, label);
    util_strcat(&as->root, str);
    free(str);

    asm_gen_expr(as, node->if_body);

    util_strcat(&as->root, label);
    util_strcat(&as->root, ":");
}


char *asm_str_from_node(struct Asm *as, struct Node *node)
{
    switch (node->type)
    {
    case NODE_INT: return asm_str_from_int(as, node);
    case NODE_STRING: return asm_str_from_str(as, node);
    case NODE_VARIABLE: return asm_str_from_var(as, node);
    case NODE_FUNCTION_CALL: return asm_str_from_function_call(as, node);
    case NODE_BINOP: return asm_str_from_binop(as, node);
    case NODE_IDOF: return asm_str_from_node(as, node->idof_new_expr);
    case NODE_INIT_LIST: return asm_str_from_init_list(as, node);
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
    return util_strcpy(node->string_asm_id);
}


char *asm_str_from_var(struct Asm *as, struct Node *node)
{
    struct Node *var = scope_find_variable(as->scope, node, node->error_line);

    if (var->type == NODE_VARIABLE)
        return asm_str_from_var_var(as, node);
    else if (var->type == NODE_VARIABLE_DEF)
        return asm_str_from_var_def(as, var);
    else
    {
        struct Node *literal = node_strip_to_literal(var, as->scope);
        return asm_str_from_node(as, literal);
    }

    return 0;
}


char *asm_str_from_var_var(struct Asm *as, struct Node *node)
{
    struct Node *var = scope_find_variable(as->scope, node, node->error_line);

    int offset = var->variable_stack_offset;

    if (node->variable_is_param)
        offset = node->variable_stack_offset;

    const char *tmp = "%d(%%ebp)";

    char *value = calloc(strlen(tmp) + MAX_INT_LEN + 1, sizeof(char));
    sprintf(value, tmp, offset);
    value = realloc(value, sizeof(char) * (strlen(value) + 1));

    if (node->variable_is_param && node != var)
    {
        const char *template =  "# Param struct member\n"
                                "movl %s, %%ebx\n";
        char *s = calloc(strlen(template) + strlen(value) + 1, sizeof(char));
        sprintf(s, template, value);
        util_strcat(&as->root, s);
        free(s);
        free(value);

        const char *temp = "%d(%%ebx)";
        offset = var->variable_stack_offset - node->variable_stack_offset;

        s = calloc(MAX_INT_LEN + 1, sizeof(char));
        sprintf(s, temp, offset);

        return s;
    }

    return value;
}


char *asm_str_from_var_def(struct Asm *as, struct Node *node)
{
    const char *tmp = "%d(%%ebp)";
    char *s = calloc(strlen(tmp) + MAX_INT_LEN + 1, sizeof(char));
    sprintf(s, tmp, node->variable_def_stack_offset);
    s = realloc(s, sizeof(char) * (strlen(s) + 1));
    return s;
}


char *asm_str_from_function_call(struct Asm *as, struct Node *node)
{
    const char *template = "# Get function call return value: avoiding too many memory references\n"
                           "movl %d(%%ebp), %%ecx\n";
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


char *asm_str_from_init_list(struct Asm *as, struct Node *node)
{
    const char *tmp = "%d(%%ebp)";
    char *s = calloc(strlen(tmp) + MAX_INT_LEN + 1, sizeof(char));
    sprintf(s, tmp, node->init_list_stack_offset);
    s = realloc(s, sizeof(char) * (strlen(s) + 1));
    return s;
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

            if (strcmp(&string_asm_id[1], buf) == 0)
                return true;
        }
    }

    return false;
}

