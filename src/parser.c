#include "parser.h"
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
        fprintf(stderr, "Unexpected token '%s' of type %d, expected type %d\n",
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

    parser_eat(parser, TOKEN_INT);
    return node;
}


struct Node *parser_parse_str(struct Parser *parser)
{
    struct Node *node = node_alloc(NODE_STRING);
    node->string_value = parser->curr_tok->value;

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
    parser_eat(parser, TOKEN_ID); // fn

    node->function_def_name = parser->curr_tok->value;
    parser_eat(parser, TOKEN_ID); // function name
    parser_eat(parser, TOKEN_LPAREN);
    // TODO Function params
    parser_eat(parser, TOKEN_RPAREN);

    parser_eat(parser, TOKEN_ARROW);

    node->function_def_return_type = type_from_id(parser->curr_tok->value);

    if (node->function_def_return_type == -1)
    {
        fprintf(stderr, "Unrecognized return type '%s'\n", parser->curr_tok->value);
        exit(EXIT_FAILURE);
    }

    parser_eat(parser, TOKEN_ID);
    parser_eat(parser, TOKEN_LBRACE);

    node->function_def_body = parser_parse(parser);

    parser_eat(parser, TOKEN_RBRACE);

    return node;
}


struct Node *parser_parse_return(struct Parser *parser)
{
    struct Node *node = node_alloc(NODE_RETURN);
    parser_eat(parser, TOKEN_ID);

    node->return_value = parser_parse_expr(parser);
    return node;
}


struct Node *parser_parse_variable_def(struct Parser *parser)
{
    struct Node *node = node_alloc(NODE_VARIABLE_DEF);
    parser_eat(parser, TOKEN_ID);

    char *name = parser->curr_tok->value;
    parser_eat(parser, TOKEN_ID);

    parser_eat(parser, TOKEN_COLON);
    node->variable_def_type = type_from_id(parser->curr_tok->value);

    if (node->variable_def_type == -1)
    {
        fprintf(stderr, "Unrecognized type annotation '%s'\n", parser->curr_tok->value);
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

    struct Node *node = node_alloc(NODE_VARIABLE);
    node->variable_name = variable_name;

    return node;
}


struct Node *parser_parse_function_call(struct Parser *parser)
{
    struct Node *node = node_alloc(NODE_FUNCTION_CALL);
    node->function_call_name = parser->tokens[parser->curr_idx - 1]->value;

    parser_eat(parser, TOKEN_LPAREN);
    struct Node *expr = parser_parse_expr(parser);

    node->function_call_args = realloc(node->function_call_args,
                            sizeof(struct Node*) * ++node->function_call_args_size);
    node->function_call_args[node->function_call_args_size - 1] = expr;

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


int type_from_id(const char *id)
{
    if (strcmp(id, "int") == 0)
        return NODE_INT;
    if (strcmp(id, "str") == 0)
        return NODE_STRING;

    return -1;
}

