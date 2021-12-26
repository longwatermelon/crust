#include "lexer.h"
#include "token.h"
#include <string.h>
#include <ctype.h>


struct Lexer *lexer_alloc(char *contents)
{
    struct Lexer *lexer = malloc(sizeof(struct Lexer));
    lexer->contents = contents;
    lexer->index = 0;
    lexer->current_c = contents[lexer->index];

    return lexer;
}


void lexer_free(struct Lexer *lexer)
{
    free(lexer);
}


void lexer_advance(struct Lexer *lexer)
{
    if (lexer->current_c != '\0' && lexer->index < strlen(lexer->contents))
        lexer->current_c = lexer->contents[++lexer->index];
}


char *lexer_collect_int(struct Lexer *lexer)
{
    size_t start = lexer->index;

    while (isdigit(lexer->current_c) && lexer->current_c != '\0')
        lexer_advance(lexer);

    char *substr = malloc(sizeof(char) * (lexer->index - start + 1));
    memcpy(substr, &lexer->contents[start], lexer->index - start);
    substr[lexer->index - start] = '\0';

    return substr;
}


char *lexer_collect_id(struct Lexer *lexer)
{
    size_t start = lexer->index;

    while (isalnum(lexer->current_c) && lexer->current_c != '\0')
        lexer_advance(lexer);

    char *substr = malloc(sizeof(char) * (lexer->index - start + 1));
    memcpy(substr, &lexer->contents[start], lexer->index - start);
    substr[lexer->index - start] = '\0';

    return substr;
}


struct Token *lexer_get_next_token(struct Lexer *lexer)
{
    while (lexer->index < strlen(lexer->contents))
    {
        while (isspace(lexer->current_c) && lexer->current_c != '\n')
            lexer_advance(lexer);

        if (isdigit(lexer->current_c))
            return token_alloc(TOKEN_INT, lexer_collect_int(lexer));

        if (isalnum(lexer->current_c))
            return token_alloc(TOKEN_ID, lexer_collect_id(lexer));

        switch (lexer->current_c)
        {
        case ';': lexer_advance(lexer); return token_alloc(TOKEN_SEMI, make_dyn_str(";"));
        case '(': lexer_advance(lexer); return token_alloc(TOKEN_LPAREN, make_dyn_str("("));
        case ')': lexer_advance(lexer); return token_alloc(TOKEN_RPAREN, make_dyn_str(")"));
        case '{': lexer_advance(lexer); return token_alloc(TOKEN_LBRACE, make_dyn_str("{"));
        case '}': lexer_advance(lexer); return token_alloc(TOKEN_RBRACE, make_dyn_str("}"));
        case '-':
            lexer_advance(lexer);
            if (lexer->current_c == '>')
                return token_alloc(TOKEN_ARROW, make_dyn_str("->"));
            break;
        }

        lexer_advance(lexer);
        ++lexer->line_num;
    }

    return token_alloc(TOKEN_EOF, make_dyn_str(""));
}


char *make_dyn_str(const char *s)
{
    char *str = malloc(sizeof(char) * (strlen(s) + 1));
    strcpy(str, s);
    return str;
}

