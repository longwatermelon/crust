#include "node.h"
#include "scope.h"
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
    node->function_def_return_type = 0;

    node->int_value = 0;

    node->string_value = 0;
    node->string_asm_id = 0;

    node->return_value = 0;

    node->variable_def_value = 0;
    node->variable_def_name = 0;
    node->variable_def_type = 0;
    node->variable_def_stack_offset = 0;

    node->variable_name = 0;
    node->variable_struct_member = 0;

    node->function_call_name = 0;
    node->function_call_args = 0;
    node->function_call_args_size = 0;

    node->param_name = 0;
    node->param_type = 0;
    node->param_stack_offset = 0;

    node->assignment_dst = 0;
    node->assignment_src = 0;

    node->struct_members = 0;
    node->struct_members_size = 0;
    node->struct_name = 0;

    node->member_name = 0;
    node->member_type = 0;

    node->init_list_values = 0;
    node->init_list_len = 0;
    node->init_list_struct_type = 0;

    node->error_line = 0;

    return node;
}


void node_free(struct Node *node)
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

    if (node->function_def_body) node_free(node->function_def_body);
    if (node->return_value) node_free(node->return_value);
    if (node->variable_def_value) node_free(node->variable_def_value);
    if (node->assignment_dst) node_free(node->assignment_dst);
    if (node->assignment_src) node_free(node->assignment_src);
    if (node->variable_struct_member) node_free(node->variable_struct_member);

    if (node->string_value) free(node->string_value);
    if (node->string_asm_id) free(node->string_asm_id);
    if (node->function_def_name) free(node->function_def_name);
    if (node->variable_def_name) free(node->variable_def_name);
    if (node->variable_name) free(node->variable_name);
    if (node->function_call_name) free(node->function_call_name);
    if (node->param_name) free(node->param_name);
    if (node->struct_name) free(node->struct_name);
    if (node->member_name) free(node->member_name);

    free(node);
}


struct Node *node_strip_to_literal(struct Node *node, struct Scope *scope)
{
    switch (node->type)
    {
    case NODE_INT:
    case NODE_STRING:
        return node;
    case NODE_VARIABLE:
        return node_strip_to_literal(scope_find_variable(scope, node), scope);
    case NODE_VARIABLE_DEF:
        return node_strip_to_literal(node->variable_def_value, scope);
    default: return node;
    }
}


char *node_str_from_type(int type)
{
    switch (type)
    {
    case NODE_INT: return "int";
    case NODE_STRING: return "str";
    default: return 0;
    }
}


int node_type_from_str(char *str)
{
    if (strcmp(str, "int") == 0)
        return NODE_INT;
    if (strcmp(str, "str") == 0)
        return NODE_STRING;

    return NODE_STRUCT;
}


int node_type_from_node(struct Node *node, struct Scope *scope)
{
    switch (node->type)
    {
    case NODE_INT:
    case NODE_STRING:
        return node->type;
    case NODE_RETURN:
        return node_type_from_node(node->return_value, scope);
    case NODE_VARIABLE:
    {
        struct Node *def = scope_find_variable(scope, node);
        return node_type_from_node(def, scope);
    } break;
    case NODE_VARIABLE_DEF:
        return node_type_from_node(node_strip_to_literal(node->variable_def_value, scope), scope);
    case NODE_PARAMETER:
        return node->param_type;
    case NODE_FUNCTION_DEF:
        return node->function_def_return_type;
    case NODE_FUNCTION_CALL:
        return scope_find_function(scope, node->function_call_name)->function_def_return_type;
    case NODE_STRUCT_MEMBER:
        return node->member_type;
    case NODE_STRUCT:
        return NODE_STRUCT;
    case NODE_INIT_LIST:
        return NODE_STRUCT;
    default: return -1;
    }
}


char *node_struct_type_from_node(struct Node *node)
{
    switch (node->type)
    {
    case NODE_STRUCT:
        return node->struct_name;
    case NODE_INIT_LIST:
        return node->init_list_struct_type;
    default: return 0;
    }
}

