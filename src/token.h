#ifndef TOKEN_H
#define TOKEN_H

#include <stdlib.h>

struct Token
{
    enum
    {
        TOKEN_ID,
        TOKEN_LPAREN,
        TOKEN_RPAREN,
        TOKEN_ARROW,
        TOKEN_LBRACE,
        TOKEN_RBRACE,
        TOKEN_SEMI,
        TOKEN_INT,
        TOKEN_EQUALS,
        TOKEN_COMMA,
        TOKEN_COLON,
        TOKEN_STRING,
        TOKEN_PERIOD,
        TOKEN_BINOP,
        TOKEN_EOF
    } type;

    char *value;
    size_t line_num;

    enum
    {
        TOKEN_OP_PLUS,
        TOKEN_OP_MINUS
    } binop_type;
};

struct Token *token_alloc(int type, char *value, size_t line_num);
void token_free(struct Token *token);

char *token_str_from_type(int type);

#endif

