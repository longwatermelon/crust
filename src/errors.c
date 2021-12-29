#include "errors.h"

#include <stdio.h>
#include <stdlib.h>

#define WHITE "\e[0;37m"
#define RED_BOLD "\e[1;31m"

void errors_check_function_call(struct Node *def, struct Node *call)
{
    if (def->function_def_params_size != call->function_call_args_size)
    {
        fprintf(stderr, RED_BOLD "Error: " WHITE "Line %lu: Function '%s' takes "
                        "%lu arguments but %lu were provided.\n", call->error_line,
                        def->function_def_name, def->function_def_params_size,
                        call->function_call_args_size);
        exit(EXIT_FAILURE);
    }
}

