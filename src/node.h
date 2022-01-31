#ifndef NODE_H
#define NODE_H

#include <stdlib.h>
#include <stdbool.h>

struct Scope;

typedef struct
{
    int type;
    char *struct_type;
} NodeDType;

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
        NODE_ASSIGNMENT,
        NODE_STRUCT,
        NODE_STRUCT_MEMBER,
        NODE_INIT_LIST,
        NODE_NOOP,
        NODE_INCLUDE,
        NODE_BINOP,
        NODE_IDOF,
        NODE_INLINE_ASM
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
    NodeDType function_def_return_type;

    struct Node **function_def_params;
    size_t function_def_params_size;

    bool function_def_is_decl;

    // Return
    struct Node *return_value;

    // Variable def
    struct Node *variable_def_value;
    char *variable_def_name;
    NodeDType variable_def_type;
    int variable_def_stack_offset;

    // Variable
    char *variable_name;
    struct Node *variable_struct_member;
    NodeDType variable_type;
    int variable_stack_offset;
    bool variable_is_param;

    // Function call
    char *function_call_name;
    struct Node **function_call_args;
    size_t function_call_args_size;
    int function_call_return_stack_offset;

    // Assignment
    struct Node *assignment_dst, *assignment_src;

    // Struct
    char *struct_name;
    struct Node **struct_members;
    size_t struct_members_size;

    // Struct member
    char *member_name;
    NodeDType member_type;

    // Initializer list
    struct Node **init_list_values;
    size_t init_list_len;
    NodeDType init_list_type;
    int init_list_stack_offset;

    // Include
    char *include_path;
    struct Node *include_root;
    struct Scope *include_scope;

    // Binop
    struct Node *op_l, *op_r;
    enum { OP_PLUS, OP_MINUS, OP_MUL, OP_DIV } op_type;
    int op_stack_offset;

    // Idof
    struct Node *idof_original_expr, *idof_new_expr;

    // Inline asm
    struct Node **asm_args;
    size_t asm_nargs;

    // Error values
    size_t error_line;
};

struct Node *node_alloc(int type);
void node_free(struct Node *node);
void node_free_lists(struct Node *node);
void node_free_strings(struct Node *node);
void node_free_dtypes(struct Node *node);

struct Node *node_strip_to_literal(struct Node *node, struct Scope *scope);

char *node_str_from_type(NodeDType type);
NodeDType node_type_from_str(char *str);

NodeDType node_type_from_node(struct Node *node, struct Scope *scope);

// Node type enum to string
char *node_str_from_node_type(int type);

bool node_dtype_cmp(NodeDType d1, NodeDType d2);
// Only compares two nodes of the same type; supports int, string, variable.
// Types other than int, str, variable shouldn't have duplicates existing in their scope,
// so false will automatically be returned.
bool node_cmp(struct Node *n1, struct Node *n2);

// Check if target exists under a subnode of parameter node.
bool node_find_node(struct Node *node, struct Node *target);

size_t node_sizeof_dtype(struct Node *node);

struct Node *node_copy(struct Node *src);
NodeDType node_dtype_copy(NodeDType src);

int node_stack_offset(struct Node *var);

#endif

