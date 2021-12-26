#ifndef LEXER_H
#define LEXER_H

#include "token.h"
#include <stdlib.h>

struct Lexer
{
    char *contents;
    size_t index;
    char current_c;

    size_t line_num;
};

struct Lexer *lexer_alloc(char *contents);
void lexer_free(struct Lexer *lexer);

void lexer_advance(struct Lexer *lexer);

char *lexer_collect_int(struct Lexer *lexer);
char *lexer_collect_id(struct Lexer *lexer);

struct Token *lexer_get_next_token(struct Lexer *lexer);

char *make_dyn_str(const char *s);

#endif

