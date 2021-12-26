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


struct Node *parser_parse_id(struct Parser *parser)
{
    if (strcmp(parser->curr_tok->value, "fn") == 0)
        return parser_parse_function_def(parser);
    else if (strcmp(parser->curr_tok->value, "return") == 0)
        return parser_parse_return(parser);
    else
    {
        fprintf(stderr, "Unrecognized identifier '%s'\n", parser->curr_tok->value);
        exit(EXIT_FAILURE);
    }
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

    if (strcmp(parser->curr_tok->value, "int") == 0)
        node->function_def_return_type = NODE_INT;
    else
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

