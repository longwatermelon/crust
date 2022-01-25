#include "token.h"
#include <stdlib.h>

struct Token *token_alloc(int type, char *value, size_t line_num)
{
    struct Token *t = malloc(sizeof(struct Token));
    t->type = type;
    t->value = value;
    t->line_num = line_num;

    return t;
}


void token_free(struct Token *token)
{
    free(token->value);
    free(token);
}


char *token_str_from_type(int type)
{
    switch (type)
    {
    case TOKEN_ID: return "identifier";
    case TOKEN_LPAREN: return "(";
    case TOKEN_RPAREN: return ")";
    case TOKEN_ARROW: return "->";
    case TOKEN_LBRACE: return "{";
    case TOKEN_RBRACE: return "}";
    case TOKEN_SEMI: return ";";
    case TOKEN_INT: return "integer";
    case TOKEN_EQUALS: return "=";
    case TOKEN_COMMA: return ",";
    case TOKEN_COLON: return ":";
    case TOKEN_STRING: return "string";
    case TOKEN_PERIOD: return ".";
    case TOKEN_EOF: return "EOF";
    }

    return 0;
}

