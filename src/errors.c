#include "errors.h"
#include "util.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RESET "\x1b[0m"
#define RED_BOLD "\x1b[1;31m"

#define ERROR RED_BOLD "Error: " RESET

#define ERROR_RANGE 1

void errors_check_function_call(struct Node *def, struct Node *call, struct Asm *as)
{
    if (def->function_def_params_size != call->function_call_args_size)
    {
        fprintf(stderr, ERROR "Function '%s' takes "
                        "%lu arguments but %lu were provided.\n",
                        def->function_def_name, def->function_def_params_size,
                        call->function_call_args_size);
        errors_print_lines(as, call->error_line, ERROR_RANGE);
        exit(EXIT_FAILURE);
    }

    for (size_t i = 0; i < call->function_call_args_size; ++i)
    {
        NodeDType type = node_type_from_node(call->function_call_args[i], as->scope);

        if (!node_dtype_cmp(type, def->function_def_params[i]->param_type))
        {
            fprintf(stderr, ERROR "Parameter %lu of function '%s' is of type %s but "
                            "data of type %s was passed.\n", i,
                            def->function_def_name, node_str_from_type(def->function_def_params[i]->param_type),
                            node_str_from_type(type));
            errors_print_lines(as, call->error_line, ERROR_RANGE);
            exit(EXIT_FAILURE);
        }
    }
}


void errors_check_function_return(struct Node *def, struct Asm *as)
{
    struct Node *comp = def->function_def_body;
    bool found_return = false;

    for (size_t i = 0; i < comp->compound_size; ++i)
    {
        struct Node *node = comp->compound_nodes[i];

        if (node->type == NODE_RETURN)
        {
            found_return = true;
            NodeDType type = node_type_from_node(node->return_value, as->scope);

            if (!node_dtype_cmp(type, def->function_def_return_type))
            {
                fprintf(stderr, ERROR "Mismatched return types; Function "
                                "'%s' has a return type of %s, but returns type %s.\n",
                                def->function_def_name,
                                node_str_from_type(def->function_def_return_type),
                                node_str_from_type(type));
                errors_print_lines(as, node->error_line, ERROR_RANGE);
                exit(EXIT_FAILURE);
            }
        }
    }

    if (def->function_def_return_type.type != NODE_NOOP && !found_return)
    {
        fprintf(stderr, ERROR "Non-void function '%s' should return '%s' but returns nothing.\n",
                        def->function_def_name, node_str_from_type(def->function_def_return_type));
        errors_print_lines(as, def->error_line, ERROR_RANGE);
        exit(EXIT_FAILURE);
    }
}


void errors_check_function_def(struct Node *def, struct Asm *as)
{
    struct Node *existing = scope_find_function(as->scope, def->function_def_name);

    if (existing)
    {
        fprintf(stderr, ERROR "Redefining function '%s'.\n", def->function_def_name);
        errors_print_lines(as, def->error_line, ERROR_RANGE);
        exit(EXIT_FAILURE);
    }
}


void errors_check_variable_def(struct Node *def, struct Asm *as)
{
    if (!node_dtype_cmp(def->variable_def_type, node_type_from_node(def->variable_def_value, as->scope)))
    {
        fprintf(stderr, ERROR "Attempting to assign value of type %s to variable "
                        "'%s' of type %s.\n",
                        node_str_from_type(node_type_from_node(def->variable_def_value, as->scope)),
                        def->variable_def_name, node_str_from_type(def->variable_def_type));
        errors_print_lines(as, def->error_line, ERROR_RANGE);
        exit(EXIT_FAILURE);
    }

    struct Node *orig = 0;
    bool duplicate = false;

    for (size_t i = 0; i < as->scope->curr_layer->variable_defs_size; ++i)
    {
        if (strcmp(as->scope->curr_layer->variable_defs[i]->variable_def_name, def->variable_def_name) == 0)
        {
            if (orig)
            {
                duplicate = true;
                break;
            }

            orig = as->scope->curr_layer->variable_defs[i];
        }
    }

    if (duplicate)
    {
        fprintf(stderr, ERROR "Attempting to redefine variable '%s'.\n",
                        def->variable_def_name);
        errors_print_lines(as, def->error_line, ERROR_RANGE);

        fprintf(stderr, "\nFirst defined here:\n");
        errors_print_lines(as, orig->error_line, ERROR_RANGE);
        exit(EXIT_FAILURE);
    }
}


void errors_check_assignment(struct Node *assignment, struct Asm *as)
{
    NodeDType src_type = node_type_from_node(assignment->assignment_src, as->scope);
    NodeDType dst_type = node_type_from_node(assignment->assignment_dst, as->scope);

    if (!node_dtype_cmp(src_type, dst_type))
    {
        fprintf(stderr, ERROR "Attempting to assign value of type %s to variable "
                        "'%s' of type %s.\n", node_str_from_type(src_type),
                        assignment->assignment_dst->variable_name,
                        node_str_from_type(dst_type));
        errors_print_lines(as, assignment->error_line, ERROR_RANGE);
        exit(EXIT_FAILURE);
    }
}


void errors_print_lines(struct Asm *as, size_t line, size_t range)
{
    for (size_t i = line - range; i <= line + range; ++i)
    {
        if (i == line)
            printf("\x1b[1;37m");

        printf("  ");
        errors_print_line(as, i);

        if (i == line)
            printf(RESET);
    }
}


void errors_print_line(struct Asm *as, size_t line)
{
    char *line_num = util_int_to_str(line);
    const char *tmp = "%s | %s";

    size_t len = strlen(tmp) + strlen(as->source[line - 1]) + strlen(line_num);
    char *s = calloc(len + 1, sizeof(char));
    sprintf(s, tmp, line_num, as->source[line - 1]);
    printf("%s", s);

    free(s);
    free(line_num);
}

