#include "errors.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define WHITE "\e[0;37m"
#define RED_BOLD "\e[1;31m"

#define ERROR RED_BOLD "Error: " WHITE
#define ON_LINE "Line %lu: "

void errors_check_function_call(struct Node *def, struct Node *call, struct Scope *scope)
{
    if (def->function_def_params_size != call->function_call_args_size)
    {
        fprintf(stderr, ERROR ON_LINE "Function '%s' takes "
                        "%lu arguments but %lu were provided.\n", call->error_line,
                        def->function_def_name, def->function_def_params_size,
                        call->function_call_args_size);
        exit(EXIT_FAILURE);
    }

    for (size_t i = 0; i < call->function_call_args_size; ++i)
    {
        int type = node_type_from_node(call->function_call_args[i], scope);

        if (type != def->function_def_params[i]->param_type)
        {
            fprintf(stderr, ERROR ON_LINE "Parameter %lu of function '%s' is of type %s but "
                            "data of type %s was passed.\n", call->error_line, i,
                            def->function_def_name, node_str_from_type(def->function_def_params[i]->param_type),
                            node_str_from_type(type));
            exit(EXIT_FAILURE);
        }
    }
}


void errors_check_function_return(struct Node *def, struct Scope *scope)
{
    struct Node *comp = def->function_def_body;
    bool found_return = false;

    for (size_t i = 0; i < comp->compound_size; ++i)
    {
        struct Node *node = comp->compound_nodes[i];

        if (node->type == NODE_RETURN)
        {
            found_return = true;
            int type = node_type_from_node(node->return_value, scope);

            if (type != def->function_def_return_type)
            {
                fprintf(stderr, ERROR ON_LINE "Mismatched return types; Function "
                                "'%s' has a return type of %s, but returns type %s.\n",
                                node->error_line, def->function_def_name,
                                node_str_from_type(def->function_def_return_type),
                                node_str_from_type(type));
                exit(EXIT_FAILURE);
            }
        }
    }

    if (!found_return)
    {
        fprintf(stderr, ERROR "Non-void function '%s' does not return a value.\n",
                        def->function_def_name);
    }
}

