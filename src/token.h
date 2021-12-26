#ifndef TOKEN_H
#define TOKEN_H

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
        TOKEN_EOF
    } type;

    char *value;
};

struct Token *token_alloc(int type, char *value);
void token_free(struct Token *token);

#endif

