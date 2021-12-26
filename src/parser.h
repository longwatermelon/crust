#ifndef PARSER_H
#define PARSER_H

#include "token.h"
#include "node.h"
#include <stdlib.h>

struct Parser
{
    struct Token **tokens;
    size_t ntokens;

    struct Token *curr_tok;
    size_t curr_idx;
};

struct Parser *parser_alloc(struct Token **tokens, size_t ntokens);
void parser_free(struct Parser *parser);

void parser_eat(struct Parser *parser, int type);

struct Node *parser_parse(struct Parser *parser);
struct Node *parser_parse_expr(struct Parser *parser);

struct Node *parser_parse_int(struct Parser *parser);
struct Node *parser_parse_id(struct Parser *parser);

struct Node *parser_parse_function_def(struct Parser *parser);
struct Node *parser_parse_return(struct Parser *parser);

#endif

