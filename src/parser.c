#include "parser.h"
#include "util.h"
#include "errors.h"
#include "crust.h"

#include <stdio.h>
#include <string.h>


struct Parser *parser_alloc(struct Token **tokens, size_t ntokens, struct Args *args)
{
    struct Parser *parser = malloc(sizeof(struct Parser));
    parser->tokens = tokens;
    parser->ntokens = ntokens;

    parser->curr_idx = 0;
    parser->curr_tok = parser->tokens[parser->curr_idx];

    parser->scope = scope_alloc();
    scope_push_layer(parser->scope);

    parser->stack_size = 4;
    parser->lc = 0;

    parser->args = args;

    return parser;
}


void parser_free(struct Parser *parser)
{
    scope_free(parser->scope);
    free(parser);
}


void parser_eat(struct Parser *parser, int type)
{
    if (parser->curr_tok->type != type)
    {
        errors_parser_unexpected_token(type, parser->curr_tok);
    }
    else
    {
        if (parser->curr_idx + 1 < parser->ntokens)
            parser->curr_tok = parser->tokens[++parser->curr_idx];
    }
}


struct Node *parser_parse(struct Parser *parser)
{
    struct Node *root = node_alloc(NODE_COMPOUND);
    root->compound_nodes = malloc(sizeof(struct Node*));
    root->compound_nodes[0] = parser_parse_expr(parser);

    if (root->compound_nodes[0])
        ++root->compound_size;

    while (parser->curr_idx < parser->ntokens)
    {
        parser_eat(parser, TOKEN_SEMI);

        struct Node *expr = parser_parse_expr(parser);

        if (!expr)
            break;

        root->compound_nodes = realloc(root->compound_nodes,
                                   sizeof(struct Node*) * ++root->compound_size);
        root->compound_nodes[root->compound_size - 1] = expr;
    }

    return root;
}


struct Node *parser_parse_expr(struct Parser *parser)
{
    switch (parser->curr_tok->type)
    {
    case TOKEN_INT: return parser_parse_int(parser);
    case TOKEN_STRING: return parser_parse_str(parser);
    case TOKEN_ID: return parser_parse_id(parser);
    default: return 0;
    }
}


struct Node *parser_parse_int(struct Parser *parser)
{
    struct Node *node = node_alloc(NODE_INT);
    node->int_value = atoi(parser->curr_tok->value);
    node->error_line = parser->curr_tok->line_num;

    parser_eat(parser, TOKEN_INT);
    return node;
}


struct Node *parser_parse_str(struct Parser *parser)
{
    struct Node *node = node_alloc(NODE_STRING);
    node->string_value = util_strcpy(parser->curr_tok->value);
    node->error_line = parser->curr_tok->line_num;

    char *label = util_int_to_str(parser->lc);
    node->string_asm_id = calloc(strlen(label) + strlen(".LC") + 1, sizeof(char));
    sprintf(node->string_asm_id, ".LC%s", label);

    ++parser->lc;
    free(label);

    parser_eat(parser, TOKEN_STRING);
    return node;
}


struct Node *parser_parse_id(struct Parser *parser)
{
    if (strcmp(parser->curr_tok->value, "fn") == 0)
        return parser_parse_function_def(parser);
    else if (strcmp(parser->curr_tok->value, "return") == 0)
        return parser_parse_return(parser);
    else if (strcmp(parser->curr_tok->value, "let") == 0)
        return parser_parse_variable_def(parser);
    else if (strcmp(parser->curr_tok->value, "struct") == 0)
        return parser_parse_struct(parser);
    else if (strcmp(parser->curr_tok->value, "include") == 0)
        return parser_parse_include(parser);
    else
        return parser_parse_variable(parser);
}


struct Node *parser_parse_function_def(struct Parser *parser)
{
    struct Node *node = node_alloc(NODE_FUNCTION_DEF);
    node->error_line = parser->curr_tok->line_num;
    parser_eat(parser, TOKEN_ID); // fn

    node->function_def_name = util_strcpy(parser->curr_tok->value);
    parser_eat(parser, TOKEN_ID); // function name

    node->function_def_params = parser_parse_function_def_params(parser, &node->function_def_params_size);

    parser_eat(parser, TOKEN_ARROW);

    node->function_def_return_type = parser_parse_dtype(parser);

    struct Node *def = scope_find_function(parser->scope, node->function_def_name);

    if (def)
    {
        node_free(node);
        node = def;
    }
    else
    {
        scope_add_function_def(parser->scope, node);
    }

    if (parser->curr_tok->type == TOKEN_SEMI)
    {
        node->function_def_is_decl = true;
        return node;
    }
    else
    {
        parser_eat(parser, TOKEN_LBRACE);

        size_t prev_size = parser->stack_size;
        parser->stack_size = 4;
        scope_push_layer(parser->scope);

        if (parser->curr_tok->type != TOKEN_RBRACE)
            node->function_def_body = parser_parse(parser);
        else
        {
            node->function_def_body = node_alloc(NODE_COMPOUND);
            node->function_def_body->compound_nodes = 0;
            node->function_def_body->compound_size = 0;
        }

        parser_eat(parser, TOKEN_RBRACE);

        scope_pop_layer(parser->scope);
        parser->stack_size = prev_size;

        if (def)
            return node_alloc(NODE_NOOP);
        else
            return node;
    }
}


struct Node **parser_parse_function_def_params(struct Parser *parser, size_t *nparams)
{
    struct Node **params = 0;
    *nparams = 0;

    parser_eat(parser, TOKEN_LPAREN);

    size_t offset = 8;

    while (parser->curr_tok->type != TOKEN_RPAREN)
    {
        struct Node *param = node_alloc(NODE_PARAMETER);
        param->error_line = parser->curr_tok->line_num;
        param->param_stack_offset = offset;
        offset += 4;

        param->param_name = util_strcpy(parser->curr_tok->value);
        parser_eat(parser, TOKEN_ID);
        parser_eat(parser, TOKEN_COLON);
        param->param_type = parser_parse_dtype(parser);

        params = realloc(params, sizeof(struct Node*) * ++*nparams);
        params[*nparams - 1] = param;

        if (parser->curr_tok->type == TOKEN_RPAREN)
            break;

        parser_eat(parser, TOKEN_COMMA);
    }

    parser_eat(parser, TOKEN_RPAREN);

    return params;
}


struct Node *parser_parse_return(struct Parser *parser)
{
    struct Node *node = node_alloc(NODE_RETURN);
    node->error_line = parser->curr_tok->line_num;
    parser_eat(parser, TOKEN_ID);

    node->return_value = parser_parse_expr(parser);
    return node;
}


struct Node *parser_parse_variable_def(struct Parser *parser)
{
    struct Node *node = node_alloc(NODE_VARIABLE_DEF);
    node->error_line = parser->curr_tok->line_num;
    parser_eat(parser, TOKEN_ID);

    char *name = util_strcpy(parser->curr_tok->value);
    parser_eat(parser, TOKEN_ID);

    parser_eat(parser, TOKEN_COLON);
    node->variable_def_type = parser_parse_dtype(parser);

    parser_eat(parser, TOKEN_EQUALS);

    node->variable_def_name = name;
    node->variable_def_value = parser_parse_expr(parser);

    node->variable_def_stack_offset = -parser->stack_size;
    parser->stack_size += node_sizeof_dtype(node_strip_to_literal(node->variable_def_value, parser->scope));

    scope_add_variable_def(parser->scope, node);

    return node;
}


struct Node *parser_parse_variable(struct Parser *parser)
{
    char *variable_name = parser->curr_tok->value;

    if (scope_find_struct(parser->scope, variable_name) &&
        parser->tokens[parser->curr_idx + 1]->type == TOKEN_LBRACE)
        return parser_parse_init_list(parser);

    parser_eat(parser, TOKEN_ID);

    if (parser->curr_tok->type == TOKEN_PERIOD)
    {
        struct Node *node = node_alloc(NODE_VARIABLE);
        node->variable_name = util_strcpy(variable_name);

        parser_eat(parser, TOKEN_PERIOD);

        node->variable_struct_member = parser_parse_variable(parser);
        return node;
    }

    if (parser->curr_tok->type == TOKEN_LPAREN)
        return parser_parse_function_call(parser);
    else if (parser->curr_tok->type == TOKEN_EQUALS)
        return parser_parse_assignment(parser);

    struct Node *node = node_alloc(NODE_VARIABLE);
    node->error_line = parser->curr_tok->line_num;
    node->variable_name = util_strcpy(variable_name);

    return node;
}


struct Node *parser_parse_function_call(struct Parser *parser)
{
    struct Node *node = node_alloc(NODE_FUNCTION_CALL);
    node->error_line = parser->curr_tok->line_num;
    node->function_call_name = util_strcpy(parser->tokens[parser->curr_idx - 1]->value);

    parser_eat(parser, TOKEN_LPAREN);
    struct Node *expr = parser_parse_expr(parser);

    if (expr)
    {
        node->function_call_args = realloc(node->function_call_args,
                    sizeof(struct Node*) * ++node->function_call_args_size);
        node->function_call_args[node->function_call_args_size - 1] = expr;
    }

    while (parser->curr_tok->type == TOKEN_COMMA)
    {
        parser_eat(parser, TOKEN_COMMA);
        expr = parser_parse_expr(parser);

        node->function_call_args = realloc(node->function_call_args,
                                sizeof(struct Node*) * ++node->function_call_args_size);

        node->function_call_args[node->function_call_args_size - 1] = expr;
    }

    parser_eat(parser, TOKEN_RPAREN);

    node->function_call_return_stack_offset = -parser->stack_size;

    struct Node *func_def = scope_find_function(parser->scope, node->function_call_name);
    size_t size = 4;

    // FIX func_def is 0 for builtin language functions; any language functions that might return structs will have issues with this
    if (func_def && func_def->function_def_return_type.struct_type)
    {
        size = node_sizeof_dtype(
            scope_find_struct(parser->scope, func_def->function_def_return_type.struct_type)
        );
    }

    parser->stack_size += size;

    return node;
}


struct Node *parser_parse_assignment(struct Parser *parser)
{
    struct Node *node = node_alloc(NODE_ASSIGNMENT);
    node->error_line = parser->curr_tok->line_num;
    node->assignment_dst = node_alloc(NODE_VARIABLE);
    node->assignment_dst->variable_name = util_strcpy(parser->tokens[parser->curr_idx - 1]->value);
    parser_eat(parser, TOKEN_EQUALS);

    node->assignment_src = parser_parse_expr(parser);

    return node;
}


struct Node *parser_parse_struct(struct Parser *parser)
{
    struct Node *node = node_alloc(NODE_STRUCT);
    parser_eat(parser, TOKEN_ID);

    node->error_line = parser->curr_tok->line_num;

    node->struct_name = util_strcpy(parser->curr_tok->value);

    scope_add_struct_def(parser->scope, node);

    parser_eat(parser, TOKEN_ID);
    parser_eat(parser, TOKEN_LBRACE);

    while (parser->curr_tok->type != TOKEN_RBRACE)
    {
        struct Node *member = node_alloc(NODE_STRUCT_MEMBER);
        member->member_name = util_strcpy(parser->curr_tok->value);

        parser_eat(parser, TOKEN_ID);
        parser_eat(parser, TOKEN_COLON);

        member->member_type = parser_parse_dtype(parser);

        node->struct_members = realloc(node->struct_members,
                        sizeof(struct Node*) * ++node->struct_members_size);
        node->struct_members[node->struct_members_size - 1] = member;

        if (parser->curr_tok->type == TOKEN_COMMA)
            parser_eat(parser, TOKEN_COMMA);
    }

    parser_eat(parser, TOKEN_RBRACE);
    return node;
}


struct Node *parser_parse_init_list(struct Parser *parser)
{
    struct Node *node = node_alloc(NODE_INIT_LIST);
    node->init_list_type = parser_parse_dtype(parser);
    node->error_line = parser->curr_tok->line_num;

    parser_eat(parser, TOKEN_LBRACE);

    while (parser->curr_tok->type != TOKEN_RBRACE)
    {
        struct Node *expr = parser_parse_expr(parser);

        node->init_list_values = realloc(node->init_list_values,
                        sizeof(struct Node*) * ++node->init_list_len);
        node->init_list_values[node->init_list_len - 1] = expr;

        if (parser->curr_tok->type == TOKEN_COMMA)
            parser_eat(parser, TOKEN_COMMA);
    }

    parser_eat(parser, TOKEN_RBRACE);
    return node;
}


struct Node *parser_parse_include(struct Parser *parser)
{
    struct Node *node = node_alloc(NODE_INCLUDE);
    parser_eat(parser, TOKEN_ID);
    node->include_path = util_strcpy(parser->curr_tok->value);
    parser_eat(parser, TOKEN_STRING);

    size_t ntokens;
    struct Token **tokens = crust_tokenize(node->include_path, &ntokens);
    struct Parser *p = parser_alloc(tokens, ntokens, parser->args);
    node->include_root = parser_parse(p);

    if (!node->include_scope)
    {
        node->include_scope = scope_alloc();
        scope_push_layer(node->include_scope);
    }

    scope_combine(node->include_scope, p->scope);

    for (size_t i = 0; i < ntokens; ++i)
        token_free(tokens[i]);

    free(tokens);
    parser_free(p);

    return node;
}


NodeDType parser_parse_dtype(struct Parser *parser)
{
    char *name = util_strcpy(parser->curr_tok->value);
    NodeDType type = node_type_from_str(name);

    if (type.type != NODE_STRUCT)
        free(name);

    parser_eat(parser, TOKEN_ID);

    return type;
}

