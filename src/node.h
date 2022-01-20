#ifndef NODE_H
#define NODE_H

#include <stdlib.h>

struct Scope;

struct Node
{
    enum
    {
        NODE_COMPOUND,
        NODE_INT,
        NODE_STRING,
        NODE_FUNCTION_DEF,
        NODE_RETURN,
        NODE_VARIABLE_DEF,
        NODE_VARIABLE,
        NODE_FUNCTION_CALL,
        NODE_PARAMETER,
        NODE_ASSIGNMENT,
        NODE_STRUCT,
        NODE_STRUCT_MEMBER,
        NODE_INIT_LIST
    } type;

    // Compound
    struct Node **compound_nodes;
    size_t compound_size;

    // Int
    int int_value;

    // String
    char *string_value;
    char *string_asm_id;

    // Function def
    char *function_def_name;
    struct Node *function_def_body;
    int function_def_return_type;

    struct Node **function_def_params;
    size_t function_def_params_size;

    // Return
    struct Node *return_value;

    // Variable def
    struct Node *variable_def_value;
    char *variable_def_name;
    int variable_def_type;
    int variable_def_stack_offset;

    // Variable
    char *variable_name;

    // Function call
    char *function_call_name;
    struct Node **function_call_args;
    size_t function_call_args_size;

    // Parameter
    char *param_name;
    int param_type;
    int param_stack_offset;

    // Assignment
    struct Node *assignment_dst, *assignment_src;

    // Struct
    char *struct_name;
    struct Node **struct_members;
    size_t struct_members_size;

    // Struct member
    char *member_name;
    int member_type;

    // Initializer list
    struct Node **init_list_values;
    size_t init_list_len;
    char *init_list_struct_type;

    // Error values
    size_t error_line;
};

struct Node *node_alloc(int type);
void node_free(struct Node *node);

struct Node *node_strip_to_literal(struct Node *node, struct Scope *scope);

char *node_str_from_type(int type);
int node_type_from_str(char *str);

int node_type_from_node(struct Node *node, struct Scope *scope);

#endif

