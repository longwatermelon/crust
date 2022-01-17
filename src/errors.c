#include "errors.h"
#include "util.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RESET "\e[0m"
#define RED_BOLD "\e[1;31m"

#define ERROR RED_BOLD "Error: " RESET
#define ON_LINE "Line %lu: "

#define ERROR_RANGE 1

void errors_check_function_call(struct Node *def, struct Node *call, struct Asm *as)
{
    if (def->function_def_params_size != call->function_call_args_size)
    {
        fprintf(stderr, ERROR ON_LINE "Function '%s' takes "
                        "%lu arguments but %lu were provided.\n", call->error_line,
                        def->function_def_name, def->function_def_params_size,
                        call->function_call_args_size);
        errors_print_lines(as, call->error_line, ERROR_RANGE);
        exit(EXIT_FAILURE);
    }

    for (size_t i = 0; i < call->function_call_args_size; ++i)
    {
        int type = node_type_from_node(call->function_call_args[i], as->scope);

        if (type != def->function_def_params[i]->param_type)
        {
            fprintf(stderr, ERROR ON_LINE "Parameter %lu of function '%s' is of type %s but "
                            "data of type %s was passed.\n", call->error_line, i,
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
            int type = node_type_from_node(node->return_value, as->scope);

            if (type != def->function_def_return_type)
            {
                fprintf(stderr, ERROR ON_LINE "Mismatched return types; Function "
                                "'%s' has a return type of %s, but returns type %s.\n",
                                node->error_line, def->function_def_name,
                                node_str_from_type(def->function_def_return_type),
                                node_str_from_type(type));
                errors_print_lines(as, node->error_line, ERROR_RANGE);
                exit(EXIT_FAILURE);
            }
        }
    }

    if (!found_return)
    {
        fprintf(stderr, ERROR "Non-void function '%s' does not return a value.\n",
                        def->function_def_name);
        exit(EXIT_FAILURE);
    }
}


void errors_check_variable_def(struct Node *def, struct Asm *as)
{
    if (def->variable_def_type != node_type_from_node(def->variable_def_value, as->scope))
    {
        fprintf(stderr, ERROR ON_LINE "Attempting to assign value of type %s to variable "
                        "'%s' of type %s.\n", def->error_line,
                        node_str_from_type(node_strip_to_literal(def->variable_def_value, as->scope)->type),
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
        fprintf(stderr, ERROR ON_LINE "Attempting to redefine variable '%s'.\n",
                        def->error_line, def->variable_def_name);
        errors_print_lines(as, def->error_line, ERROR_RANGE);

        fprintf(stderr, "First defined here:\n");
        errors_print_lines(as, orig->error_line, ERROR_RANGE);
        exit(EXIT_FAILURE);
    }
}


void errors_check_assignment(struct Node *assignment, struct Asm *as)
{
    int src_type = node_type_from_node(assignment->assignment_src, as->scope);
    int dst_type = node_type_from_node(assignment->assignment_dst, as->scope);

    if (src_type != dst_type)
    {
        fprintf(stderr, ERROR ON_LINE "Attempting to assign value of type %s to variable "
                        "'%s' of type %s.\n", assignment->error_line,
                        node_str_from_type(src_type),
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
            printf("\e[1;37m");

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

