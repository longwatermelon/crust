#include "node.h"


struct Node *node_alloc(int type)
{
    struct Node *node = malloc(sizeof(struct Node));
    node->type = type;

    node->compound_nodes = 0;
    node->compound_size = 0;

    node->function_def_body = 0;
    node->function_def_name = 0;

    node->int_value = 0;

    node->return_value = 0;

    return node;
}


void node_free(struct Node *node)
{
    if (node->compound_nodes)
    {
        for (int i = 0; i < node->compound_size; ++i)
            node_free(node->compound_nodes[i]);

        free(node->compound_nodes);
    }

    if (node->function_def_body)
        node_free(node->function_def_body);

    if (node->return_value)
        node_free(node->return_value);

    free(node);
}

