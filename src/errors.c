#include "errors.h"

#include <stdio.h>
#include <stdlib.h>

#define WHITE "\e[0;37m"
#define RED_BOLD "\e[1;31m"

#define ERROR_ON_LINE RED_BOLD "Error: " WHITE "Line %lu: "

void errors_check_function_call(struct Node *def, struct Node *call, struct Scope *scope)
{
    if (def->function_def_params_size != call->function_call_args_size)
    {
        fprintf(stderr, ERROR_ON_LINE "Function '%s' takes "
                        "%lu arguments but %lu were provided.\n", call->error_line,
                        def->function_def_name, def->function_def_params_size,
                        call->function_call_args_size);
        exit(EXIT_FAILURE);
    }

    for (size_t i = 0; i < call->function_call_args_size; ++i)
    {
        struct Node *node = node_strip_to_literal(call->function_call_args[i], scope);

        int type;

        if (node->type == NODE_VARIABLE_DEF)
            type = node->variable_def_type;
        else if (node->type == NODE_PARAMETER)
            type = node->param_type;
        else
            type = node->type;

        if (type != def->function_def_params[i]->param_type)
        {
            fprintf(stderr, ERROR_ON_LINE "Parameter %lu of function '%s' is of type %s but "
                            "data of type %s was passed.\n", call->error_line, i,
                            def->function_def_name, node_str_from_type(def->function_def_params[i]->param_type),
                            node_str_from_type(type));
            exit(EXIT_FAILURE);
        }
    }
}

