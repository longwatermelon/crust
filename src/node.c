#include "node.h"
#include "scope.h"
#include "util.h"

#include <string.h>


struct Node *node_alloc(int type)
{
    struct Node *node = malloc(sizeof(struct Node));
    node->type = type;

    node->compound_nodes = 0;
    node->compound_size = 0;

    node->function_def_body = 0;
    node->function_def_name = 0;
    node->function_def_params = 0;
    node->function_def_params_size = 0;
    node->function_def_return_type = (NodeDType){ 0, 0 };
    node->function_def_is_decl = false;

    node->int_value = 0;

    node->string_value = 0;
    node->string_asm_id = 0;

    node->return_value = 0;

    node->variable_def_value = 0;
    node->variable_def_name = 0;
    node->variable_def_type = (NodeDType){ 0, 0 };
    node->variable_def_stack_offset = 0;

    node->variable_name = 0;
    node->variable_struct_member = 0;
    node->variable_type = (NodeDType){ 0, 0 };
    node->variable_stack_offset = 0;
    node->variable_is_param = false;

    node->function_call_name = 0;
    node->function_call_args = 0;
    node->function_call_args_size = 0;
    node->function_call_return_stack_offset = 0;

    node->assignment_dst = 0;
    node->assignment_src = 0;

    node->struct_members = 0;
    node->struct_members_size = 0;
    node->struct_name = 0;

    node->member_name = 0;
    node->member_type = (NodeDType){ 0, 0 };

    node->init_list_values = 0;
    node->init_list_len = 0;
    node->init_list_type = (NodeDType){ 0, 0 };
    node->init_list_stack_offset = 0;

    node->include_path = 0;
    node->include_root = 0;
    node->include_scope = 0;

    node->op_l = 0;
    node->op_r = 0;
    node->op_type = 0;
    node->op_stack_offset = 0;

    node->idof_original_expr = 0;
    node->idof_new_expr = 0;

    node->asm_args = 0;
    node->asm_nargs = 0;

    node->error_line = 0;

    return node;
}


void node_free(struct Node *node)
{
    node_free_lists(node);
    node_free_strings(node);
    node_free_dtypes(node);

    if (node->function_def_body) node_free(node->function_def_body);
    if (node->return_value) node_free(node->return_value);
    if (node->variable_def_value) node_free(node->variable_def_value);
    if (node->assignment_dst) node_free(node->assignment_dst);
    if (node->assignment_src) node_free(node->assignment_src);
    if (node->variable_struct_member) node_free(node->variable_struct_member);
    if (node->include_root) node_free(node->include_root);
    if (node->include_scope) scope_free(node->include_scope);
    if (node->op_l) node_free(node->op_l);
    if (node->op_r) node_free(node->op_r);
    if (node->idof_original_expr) node_free(node->idof_original_expr);
    if (node->idof_new_expr) node_free(node->idof_new_expr);

    free(node);
}


void node_free_lists(struct Node *node)
{
    if (node->compound_nodes)
    {
        for (size_t i = 0; i < node->compound_size; ++i)
            node_free(node->compound_nodes[i]);

        free(node->compound_nodes);
    }

    if (node->function_call_args)
    {
        for (size_t i = 0; i < node->function_call_args_size; ++i)
            node_free(node->function_call_args[i]);

        free(node->function_call_args);
    }

    if (node->function_def_params)
    {
        for (size_t i = 0; i < node->function_def_params_size; ++i)
            node_free(node->function_def_params[i]);

        free(node->function_def_params);
    }

    if (node->struct_members)
    {
        for (size_t i = 0; i < node->struct_members_size; ++i)
            node_free(node->struct_members[i]);

        free(node->struct_members);
    }

    if (node->init_list_values)
    {
        for (size_t i = 0; i < node->init_list_len; ++i)
            node_free(node->init_list_values[i]);

        free(node->init_list_values);
    }

    if (node->asm_args)
    {
        for (size_t i = 0; i < node->asm_nargs; ++i)
            node_free(node->asm_args[i]);

        free(node->asm_args);
    }
}


void node_free_strings(struct Node *node)
{
    if (node->string_value) free(node->string_value);
    if (node->string_asm_id) free(node->string_asm_id);
    if (node->function_def_name) free(node->function_def_name);
    if (node->variable_def_name) free(node->variable_def_name);
    if (node->variable_name) free(node->variable_name);
    if (node->function_call_name) free(node->function_call_name);
    if (node->struct_name) free(node->struct_name);
    if (node->member_name) free(node->member_name);
    if (node->include_path) free(node->include_path);
}


void node_free_dtypes(struct Node *node)
{
    if (node->function_def_return_type.struct_type) free(node->function_def_return_type.struct_type);
    if (node->variable_def_type.struct_type) free(node->variable_def_type.struct_type);
    if (node->member_type.struct_type) free(node->member_type.struct_type);
    if (node->init_list_type.struct_type) free(node->init_list_type.struct_type);
    if (node->variable_type.struct_type) free(node->variable_type.struct_type);
}


struct Node *node_strip_to_literal(struct Node *node, struct Scope *scope)
{
    switch (node->type)
    {
    case NODE_INT:
    case NODE_STRING:
        return node;
    case NODE_VARIABLE:
    {
        if (node->variable_struct_member)
            return node;

        struct Node *var = scope_find_variable(scope, node, node->error_line);

        if (var->type == NODE_VARIABLE)
            return var;
        else
            return node_strip_to_literal(var, scope);
    } break;
    case NODE_VARIABLE_DEF:
        return node_strip_to_literal(node->variable_def_value, scope);
    case NODE_IDOF:
        return node_strip_to_literal(node->idof_new_expr, scope);
    default: return node;
    }
}


char *node_str_from_type(NodeDType type)
{
    switch (type.type)
    {
    case NODE_INT: return "int";
    case NODE_STRING: return "str";
    case NODE_NOOP: return "void";
    default: return type.struct_type;
    }
}


NodeDType node_type_from_str(char *str)
{
    if (strcmp(str, "int") == 0)
        return (NodeDType){ NODE_INT, 0 };
    if (strcmp(str, "str") == 0)
        return (NodeDType){ NODE_STRING, 0 };
    if (strcmp(str, "void") == 0)
        return (NodeDType){ NODE_NOOP, 0 };

    return (NodeDType){ NODE_STRUCT, str };
}


NodeDType node_type_from_node(struct Node *node, struct Scope *scope)
{
    switch (node->type)
    {
    case NODE_INT:
    case NODE_STRING:
        return (NodeDType){ node->type, 0 };

    case NODE_RETURN:
        return node_type_from_node(node->return_value, scope);

    case NODE_VARIABLE:
    {
        if (node->variable_struct_member)
            return node_type_from_node(node->variable_struct_member, scope);

        return node->variable_type;
    } break;

    case NODE_VARIABLE_DEF:
        return node_type_from_node(node_strip_to_literal(node->variable_def_value, scope), scope);

    case NODE_FUNCTION_DEF:
        return node->function_def_return_type;

    case NODE_FUNCTION_CALL:
        return scope_find_function(scope, node->function_call_name, node->error_line)->function_def_return_type;

    case NODE_STRUCT_MEMBER:
        return node->member_type;

    case NODE_STRUCT:
        return (NodeDType){ NODE_STRUCT, node->struct_name };

    case NODE_INIT_LIST:
        return node->init_list_type;

    case NODE_BINOP:
        return node_type_from_node(node->op_l, scope);

    case NODE_IDOF:
        return (NodeDType){ NODE_STRING, 0 };

    default: return (NodeDType){ 0, 0 };
    }
}


char *node_str_from_node_type(int type)
{
    switch (type)
    {
    case NODE_COMPOUND: return "compound";
    case NODE_INT: return "integer";
    case NODE_STRING: return "string";
    case NODE_FUNCTION_DEF: return "function def";
    case NODE_RETURN: return "return";
    case NODE_VARIABLE_DEF: return "variable def";
    case NODE_VARIABLE: return "variable";
    case NODE_FUNCTION_CALL: return "function call";
    case NODE_ASSIGNMENT: return "assignment";
    case NODE_STRUCT: return "struct";
    case NODE_STRUCT_MEMBER: return "member";
    case NODE_INIT_LIST: return "init list";
    case NODE_NOOP: return "noop";
    case NODE_INCLUDE: return "include";
    case NODE_BINOP: return "binop";
    case NODE_IDOF: return "idof";
    case NODE_INLINE_ASM: return "asm";
    }

    return 0;
}


bool node_dtype_cmp(NodeDType d1, NodeDType d2)
{
    if (d1.type == d2.type)
    {
        if (d1.struct_type && d2.struct_type)
            return strcmp(d1.struct_type, d2.struct_type) == 0;

        return true;
    }

    return false;
}


bool node_cmp(struct Node *n1, struct Node *n2)
{
    switch (n1->type)
    {
    case NODE_INT: return n1->int_value == n2->int_value;
    case NODE_STRING: return strcmp(n1->string_value, n2->string_value) == 0;
    case NODE_VARIABLE:
        if (n1->variable_struct_member && n2->variable_struct_member)
            return node_cmp(n1->variable_struct_member, n2->variable_struct_member);

        return strcmp(n1->variable_name, n2->variable_name) == 0;
    case NODE_FUNCTION_CALL: return strcmp(n1->function_call_name, n2->function_call_name) == 0;
    default: return false;
    }
}


bool node_find_node(struct Node *node, struct Node *target)
{
    if (node->type == target->type && node_cmp(node, target))
        return true;

    switch (node->type)
    {
    case NODE_COMPOUND:
        for (size_t i = 0; i < node->compound_size; ++i)
        {
            if (node_find_node(node->compound_nodes[i], target))
                return true;
        }

        break;
    case NODE_FUNCTION_DEF:
        return node_find_node(node->function_def_body, target);

        break;
    case NODE_ASSIGNMENT:
        if (node->assignment_dst == target || node_find_node(node->assignment_src, target))
            return true;

        break;
    case NODE_INIT_LIST:
        for (size_t i = 0; i < node->init_list_len; ++i)
        {
            if (node_find_node(node->init_list_values[i], target))
                return true;
        }

        break;
    case NODE_RETURN:
        return node_find_node(node->return_value, target);
        break;
    case NODE_FUNCTION_CALL:
        for (size_t i = 0; i < node->function_call_args_size; ++i)
        {
            if (node_find_node(node->function_call_args[i], target))
                return true;
        }

        break;
    case NODE_BINOP:
        if (node_find_node(node->op_l, target) || node_find_node(node->op_r, target))
            return true;

        break;
    case NODE_INLINE_ASM:
        for (size_t i = 0; i < node->asm_nargs; ++i)
        {
            if (node_find_node(node->asm_args[i], target))
                return true;
        }

        break;
    case NODE_IDOF:
        if (node_find_node(node->idof_original_expr, target) || node_find_node(node->idof_new_expr, target))
            return true;

        break;
    case NODE_VARIABLE_DEF:
        if (node_find_node(node->variable_def_value, target))
            return true;

        break;
    default: return false;
    }

    return false;
}


size_t node_sizeof_dtype(struct Node *node)
{
    switch (node->type)
    {
    case NODE_INT:
    case NODE_STRING:
    case NODE_NOOP:
        return 4;
    case NODE_STRUCT:
        return node->struct_members_size * 4;
    case NODE_INIT_LIST:
        return node->init_list_len * 4;
    default: return 0;
    }
}


struct Node *node_copy(struct Node *src)
{
    struct Node *ret = node_alloc(src->type);
    ret->error_line = src->error_line;

    switch (src->type)
    {
    case NODE_ASSIGNMENT:
        ret->assignment_dst = node_copy(src->assignment_dst);
        ret->assignment_src = node_copy(src->assignment_src);
        return ret;

    case NODE_COMPOUND:
        ret->compound_nodes = malloc(sizeof(struct Node*) * src->compound_size);
        ret->compound_size = src->compound_size;

        for (size_t i = 0; i < src->compound_size; ++i)
            ret->compound_nodes[i] = node_copy(src->compound_nodes[i]);

        return ret;

    case NODE_FUNCTION_CALL:
        ret->function_call_name = util_strcpy(src->function_call_name);
        ret->function_call_return_stack_offset = src->function_call_return_stack_offset;
        ret->function_call_args = malloc(sizeof(struct Node*) * src->function_call_args_size);
        ret->function_call_args_size = src->function_call_args_size;

        for (size_t i = 0; i < src->function_call_args_size; ++i)
            ret->function_call_args[i] = node_copy(src->function_call_args[i]);

        return ret;

    case NODE_FUNCTION_DEF:
        ret->function_def_is_decl = src->function_def_is_decl;
        ret->function_def_name = util_strcpy(src->function_def_name);

        if (!src->function_def_is_decl)
            ret->function_def_body = node_copy(src->function_def_body);

        ret->function_def_return_type = node_dtype_copy(src->function_def_return_type);

        ret->function_def_params = malloc(sizeof(struct Node*) * src->function_def_params_size);
        ret->function_def_params_size = src->function_def_params_size;

        for (size_t i = 0; i < src->function_def_params_size; ++i)
            ret->function_def_params[i] = node_copy(src->function_def_params[i]);

        return ret;

    case NODE_INCLUDE:
        ret->include_path = util_strcpy(src->include_path);
        return ret;

    case NODE_INIT_LIST:
        ret->init_list_type = node_dtype_copy(src->init_list_type);

        ret->init_list_values = malloc(sizeof(struct Node*) * src->init_list_len);
        ret->init_list_len = src->init_list_len;

        for (size_t i = 0; i < src->init_list_len; ++i)
            ret->init_list_values[i] = node_copy(src->init_list_values[i]);

        return ret;

    case NODE_INT:
        ret->int_value = src->int_value;
        return ret;

    case NODE_NOOP:
        return ret;

    case NODE_RETURN:
        ret->return_value = node_copy(src->return_value);
        return ret;

    case NODE_STRING:
        ret->string_value = util_strcpy(src->string_value);
        ret->string_asm_id = util_strcpy(src->string_asm_id);
        return ret;

    case NODE_STRUCT:
        ret->struct_name = util_strcpy(src->struct_name);

        ret->struct_members = malloc(sizeof(struct Node*) * src->struct_members_size);
        ret->struct_members_size = src->struct_members_size;

        for (size_t i = 0; i < src->struct_members_size; ++i)
            ret->struct_members[i] = node_copy(src->struct_members[i]);

        return ret;

    case NODE_STRUCT_MEMBER:
        ret->member_name = util_strcpy(src->member_name);
        ret->member_type = node_dtype_copy(src->member_type);
        return ret;

    case NODE_VARIABLE:
        ret->variable_name = util_strcpy(src->variable_name);

        if (src->variable_struct_member)
            ret->variable_struct_member = node_copy(src->variable_struct_member);

        ret->variable_type = node_dtype_copy(src->variable_type);
        ret->variable_stack_offset = src->variable_stack_offset;
        ret->variable_is_param = src->variable_is_param;

        return ret;

    case NODE_VARIABLE_DEF:
        ret->variable_def_name = util_strcpy(src->variable_def_name);
        ret->variable_def_stack_offset = src->variable_def_stack_offset;
        ret->variable_def_type = node_dtype_copy(src->variable_def_type);
        ret->variable_def_value = node_copy(src->variable_def_value);

        return ret;

    case NODE_BINOP:
        ret->op_l = node_copy(src->op_l);
        ret->op_r = node_copy(src->op_r);

        return ret;

    case NODE_IDOF:
        ret->idof_original_expr = node_copy(src->idof_original_expr);
        ret->idof_new_expr = node_copy(src->idof_new_expr);

        return ret;

    case NODE_INLINE_ASM:
        ret->asm_args = malloc(sizeof(struct Node*) * src->asm_nargs);
        ret->asm_nargs = src->asm_nargs;

        for (size_t i = 0; i < src->asm_nargs; ++i)
            ret->asm_args[i] = src->asm_args[i];

        return ret;
    }

    return 0;
}


NodeDType node_dtype_copy(NodeDType src)
{
    return (NodeDType){
        src.type,
        src.struct_type ? util_strcpy(src.struct_type) : 0
    };
}


int node_stack_offset(struct Node *var)
{
    if (var->type == NODE_VARIABLE)
        return var->variable_stack_offset;
    else if (var->type == NODE_VARIABLE_DEF)
        return var->variable_def_stack_offset;

    return 0;
}

