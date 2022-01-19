#include "parser.h"
#include "util.h"
#include <stdio.h>
#include <string.h>


struct Parser *parser_alloc(struct Token **tokens, size_t ntokens)
{
    struct Parser *parser = malloc(sizeof(struct Parser));
    parser->tokens = tokens;
    parser->ntokens = ntokens;

    parser->curr_idx = 0;
    parser->curr_tok = parser->tokens[parser->curr_idx];

    return parser;
}


void parser_free(struct Parser *parser)
{
    free(parser);
}


void parser_eat(struct Parser *parser, int type)
{
    if (parser->curr_tok->type != type)
    {
        fprintf(stderr, "Parser error: Unexpected token '%s' of type %d, expected type %d\n",
                        parser->curr_tok->value, parser->curr_tok->type, type);
        exit(EXIT_FAILURE);
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

    node->function_def_return_type = node_type_from_str(parser->curr_tok->value);

    if (node->function_def_return_type == -1)
    {
        fprintf(stderr, "Parser error: Unrecognized return type '%s'\n", parser->curr_tok->value);
        exit(EXIT_FAILURE);
    }

    parser_eat(parser, TOKEN_ID);
    parser_eat(parser, TOKEN_LBRACE);

    node->function_def_body = parser_parse(parser);

    parser_eat(parser, TOKEN_RBRACE);

    return node;
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
        param->param_type = node_type_from_str(parser->curr_tok->value);
        parser_eat(parser, TOKEN_ID);

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
    node->variable_def_type = node_type_from_str(parser->curr_tok->value);

    if (node->variable_def_type == -1)
    {
        fprintf(stderr, "Parser error: Unrecognized type annotation '%s'\n", parser->curr_tok->value);
        exit(EXIT_FAILURE);
    }

    parser_eat(parser, TOKEN_ID);

    parser_eat(parser, TOKEN_EQUALS);

    node->variable_def_name = name;
    node->variable_def_value = parser_parse_expr(parser);

    return node;
}


struct Node *parser_parse_variable(struct Parser *parser)
{
    char *variable_name = parser->curr_tok->value;
    parser_eat(parser, TOKEN_ID);

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

