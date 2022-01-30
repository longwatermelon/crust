#include "errors.h"
#include "util.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define RESET "\x1b[0m"
#define RED_BOLD "\x1b[1;31m"
#define YELLOW_BOLD "\x1b[1;33m"
#define WHITE_BOLD "\x1b[1;37m"
#define MAGENTA_BOLD "\x1b[1;35m"

#define ERROR RED_BOLD "Error: " RESET
#define INTERNAL_ERROR RED_BOLD "Internal compiler error: " RESET
#define WARNING YELLOW_BOLD "Warning: " RESET
#define WARNING_FLAG(x) "[" MAGENTA_BOLD x RESET "]"

#define ERROR_RANGE 1

char **g_source = 0;
size_t g_source_len = 0;

void errors_load_source(char **source, size_t nlines)
{
    g_source = source;
    g_source_len = nlines;
}


void errors_lexer_unrecognized_char(char c, size_t line)
{
    fprintf(stderr, ERROR "Unrecognized character '%c'.\n", c);
    errors_print_lines(line);
    exit(EXIT_FAILURE);
}


void errors_parser_unexpected_token(int expected, struct Token *found)
{
    fprintf(stderr, ERROR "Unexpected token '%s'; expected '%s'\n",
                    found->value, token_str_from_type(expected));
    errors_print_lines(found->line_num);
    exit(EXIT_FAILURE);
}


void errors_parser_idof_wrong_type(struct Node *idof_expr)
{
    fprintf(stderr, ERROR "Idof only accepts strings, but a node of type '%s' "
                    "was passed.\n", node_str_from_node_type(idof_expr->type));
    errors_print_lines(idof_expr->error_line);
    exit(EXIT_FAILURE);
}


void errors_parser_nonexistent_include(struct Node *node)
{
    fprintf(stderr, ERROR "File '%s' does not exist.\n", node->include_path);
    errors_print_lines(node->error_line);
    exit(EXIT_FAILURE);
}


void errors_asm_check_function_call(struct Scope *scope, struct Node *def, struct Node *call)
{
    if (def->function_def_params_size != call->function_call_args_size)
    {
        fprintf(stderr, ERROR "Function '%s' takes "
                        "%zu arguments but %zu were provided.\n",
                        def->function_def_name, def->function_def_params_size,
                        call->function_call_args_size);
        errors_print_lines(call->error_line);
        exit(EXIT_FAILURE);
    }

    for (size_t i = 0; i < call->function_call_args_size; ++i)
    {
        NodeDType type = node_type_from_node(call->function_call_args[i], scope);

        if (!node_dtype_cmp(type, def->function_def_params[i]->param_type))
        {
            fprintf(stderr, ERROR "Parameter %zu of function '%s' is of type %s but "
                            "data of type %s was passed.\n", i,
                            def->function_def_name, node_str_from_type(def->function_def_params[i]->param_type),
                            node_str_from_type(type));
            errors_print_lines(call->error_line);
            exit(EXIT_FAILURE);
        }
    }
}


void errors_asm_check_function_return(struct Scope *scope, struct Node *def)
{
    struct Node *comp = def->function_def_body;
    bool found_return = false;

    for (size_t i = 0; i < comp->compound_size; ++i)
    {
        struct Node *node = comp->compound_nodes[i];

        if (node->type == NODE_RETURN)
        {
            found_return = true;
            NodeDType type = node_type_from_node(node->return_value, scope);

            if (!node_dtype_cmp(type, def->function_def_return_type))
            {
                fprintf(stderr, ERROR "Mismatched return types; Function "
                                "'%s' has a return type of %s, but returns type %s.\n",
                                def->function_def_name,
                                node_str_from_type(def->function_def_return_type),
                                node_str_from_type(type));
                errors_print_lines(node->error_line);
                exit(EXIT_FAILURE);
            }
        }
    }

    if (def->function_def_return_type.type != NODE_NOOP && !found_return)
    {
        fprintf(stderr, ERROR "Non-void function '%s' should return '%s' but returns nothing.\n",
                        def->function_def_name, node_str_from_type(def->function_def_return_type));
        errors_print_lines(def->error_line);
        exit(EXIT_FAILURE);
    }
}


void errors_asm_check_function_def(struct Scope *scope, struct Node *def)
{
    struct Node *existing = scope_find_function_def(scope, def->function_def_name, -1);

    if (existing)
    {
        fprintf(stderr, ERROR "Redefining function '%s'.\n", def->function_def_name);
        errors_print_lines(def->error_line);
        exit(EXIT_FAILURE);
    }
}


void errors_asm_check_variable_def(struct Scope *scope, struct Node *def)
{
    if (!node_dtype_cmp(def->variable_def_type, node_type_from_node(def->variable_def_value, scope)))
    {
        fprintf(stderr, ERROR "Attempting to assign value of type %s to variable "
                        "'%s' of type %s.\n",
                        node_str_from_type(node_type_from_node(def->variable_def_value, scope)),
                        def->variable_def_name, node_str_from_type(def->variable_def_type));
        errors_print_lines(def->error_line);
        exit(EXIT_FAILURE);
    }

    struct Node *orig = 0;
    bool duplicate = false;

    for (size_t i = 0; i < scope->curr_layer->variable_defs_size; ++i)
    {
        if (strcmp(scope->curr_layer->variable_defs[i]->variable_def_name, def->variable_def_name) == 0)
        {
            if (orig)
            {
                duplicate = true;
                break;
            }

            orig = scope->curr_layer->variable_defs[i];
        }
    }

    if (duplicate)
    {
        fprintf(stderr, ERROR "Attempting to redefine variable '%s'.\n",
                        def->variable_def_name);
        errors_print_lines(def->error_line);

        fprintf(stderr, "\nFirst defined here:\n");
        errors_print_lines(orig->error_line);
        exit(EXIT_FAILURE);
    }
}


void errors_asm_check_assignment(struct Scope *scope, struct Node *assignment)
{
    NodeDType src_type = node_type_from_node(assignment->assignment_src, scope);
    NodeDType dst_type = node_type_from_node(assignment->assignment_dst, scope);

    if (!node_dtype_cmp(src_type, dst_type))
    {
        fprintf(stderr, ERROR "Attempting to assign value of type %s to variable "
                        "'%s' of type %s.\n", node_str_from_type(src_type),
                        assignment->assignment_dst->variable_name,
                        node_str_from_type(dst_type));
        errors_print_lines(assignment->error_line);
        exit(EXIT_FAILURE);
    }
}


void errors_asm_check_init_list(struct Scope *scope, struct Node *list)
{
    struct Node *struct_node = scope_find_struct(scope, list->init_list_type.struct_type, list->error_line);

    if (list->init_list_len != struct_node->struct_members_size)
    {
        fprintf(stderr, ERROR "Struct '%s' has %zu members but %zu member(s) were passed.\n",
                        struct_node->struct_name, struct_node->struct_members_size,
                        list->init_list_len);
        errors_print_lines(list->error_line);
        exit(EXIT_FAILURE);
    }

    for (size_t i = 0; i < list->init_list_len; ++i)
    {
        if (list->init_list_values[i]->type != struct_node->struct_members[i]->member_type.type)
        {
            fprintf(stderr, ERROR "Attempting to initialize member '%s' of type '%s' from struct '%s' "
                                  "with type '%s'.\n", struct_node->struct_members[i]->member_name,
                                  node_str_from_type(struct_node->struct_members[i]->member_type),
                                  struct_node->struct_name,
                                  node_str_from_type((NodeDType){ list->init_list_values[i]->type }));
            errors_print_lines(list->error_line);

            fprintf(stderr, "\nStruct first defined here:\n");
            errors_print_lines(struct_node->error_line);

            exit(EXIT_FAILURE);
        }
    }
}


void errors_asm_str_from_node(struct Node *node)
{
    fprintf(stderr, INTERNAL_ERROR "Unable to extract value from data of type %d.\n", node->type);
    errors_print_lines(node->error_line);
    exit(EXIT_FAILURE);
}


void errors_args_nonexistent_warning(char *warning)
{
    fprintf(stderr, ERROR "'%s' is not a valid warning option.\n", warning);
    exit(EXIT_FAILURE);
}


void errors_args_no_opt_value(char *opt)
{
    fprintf(stderr, ERROR "Expected expression after option '%s'.\n", opt);
    exit(EXIT_FAILURE);
}


void errors_scope_nonexistent_variable(char *name, size_t line)
{
    fprintf(stderr, ERROR "Variable '%s' does not exist.\n", name);
    errors_print_lines(line);
    exit(EXIT_FAILURE);
}


void errors_scope_nonexistent_function(char *name, size_t line)
{
    fprintf(stderr, ERROR "Function '%s' does not exist.\n", name);
    errors_print_lines(line);
    exit(EXIT_FAILURE);
}


void errors_scope_nonexistent_struct(char *name, size_t line)
{
    fprintf(stderr, ERROR "Struct '%s' does not exist.\n", name);
    errors_print_lines(line);
    exit(EXIT_FAILURE);
}


void errors_warn_dead_code(struct Node *func_def)
{
    if (!func_def->function_def_is_decl &&
        func_def->function_def_body->compound_size == 0)
    {
        fprintf(stderr, WARNING "'%s' is a useless function. "
                        WARNING_FLAG("-Wno-dead-code") "\n",
                        func_def->function_def_name);
        errors_print_lines(func_def->error_line);
        printf("\n");
    }
}


void errors_warn_unused_variable(struct Scope *scope, struct Node *func_def)
{
    for (size_t i = 0; i < scope->curr_layer->variable_defs_size; ++i)
    {
        struct Node *def = scope->curr_layer->variable_defs[i];

        struct Node *var = node_alloc(NODE_VARIABLE);
        var->variable_name = util_strcpy(def->variable_def_name);

        if (!node_find_node(func_def, var))
            errors_warn_print_unused_variable(def->error_line, def->variable_def_name);

        node_free(var);
    }

    for (size_t i = 0; i < func_def->function_def_params_size; ++i)
    {
        struct Node *param = func_def->function_def_params[i];

        struct Node *var = node_alloc(NODE_VARIABLE);
        var->variable_name = util_strcpy(param->param_name);

        if (!node_find_node(func_def, var))
            errors_warn_print_unused_variable(param->error_line, param->param_name);

        node_free(var);
    }
}


void errors_warn_print_unused_variable(size_t line, char *var_name)
{
    fprintf(stderr, WARNING "Variable '%s' is unused. "
                    WARNING_FLAG("-Wno-unused-variable") "\n", var_name);
    printf("Variable was first declared here:\n");
    errors_print_lines(line);
    printf("\n");
}


void errors_warn_redundant_idof(struct Node *idof)
{
    fprintf(stderr, WARNING "Redundant 'idof'; Node of type '%s' does not need idof. "
                    "Idof is only necessary for string literals and variables, excluding "
                    "parameters. " WARNING_FLAG("-Wno-redundant-idof") "\n",
                    node_str_from_node_type(idof->idof_new_expr->type));
    errors_print_lines(idof->error_line);
    printf("\n");
}


void errors_print_lines(size_t line)
{
    int begin = line - ERROR_RANGE;

    if (begin <= 0)
        begin = 1;

    int end = begin + 2 * ERROR_RANGE;

    if (end >= g_source_len)
        end = g_source_len;

    for (int i = begin; i <= end; ++i)
    {
        if (i == line)
            printf(WHITE_BOLD);

        printf("  ");
        errors_print_line(i);

        if (i == line)
            printf(RESET);
    }
}


void errors_print_line(size_t line)
{
    char *line_num = util_int_to_str(line);
    const char *tmp = "%s | %s";

    size_t len = strlen(tmp) + strlen(g_source[line - 1]) + strlen(line_num);
    char *s = calloc(len + 1, sizeof(char));
    sprintf(s, tmp, line_num, g_source[line - 1]);
    printf("%s", s);

    free(s);
    free(line_num);
}

