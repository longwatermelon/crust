#ifndef NODE_H
#define NODE_H

#include <stdlib.h>

struct Node
{
    enum
    {
        NODE_COMPOUND,
        NODE_INT,
        NODE_FUNCTION_DEF,
        NODE_RETURN,
        NODE_VARIABLE_DEF,
        NODE_VARIABLE,
        NODE_FUNCTION_CALL
    } type;

    // Compound
    struct Node **compound_nodes;
    size_t compound_size;

    // Int
    int int_value;

    // Function def
    char *function_def_name;
    struct Node *function_def_body;
    int function_def_return_type;

    // Return
    struct Node *return_value;

    // Variable def
    struct Node *variable_def_value;
    char *variable_def_name;
    int variable_def_type;

    // Variable
    char *variable_name;

    // Function call
    char *function_call_name;
    struct Node **function_call_args;
    size_t function_call_args_size;
};

struct Node *node_alloc(int type);
void node_free(struct Node *node);

#endif

