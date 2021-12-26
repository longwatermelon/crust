#include "ham.h"
#include "lexer.h"
#include "util.h"


void ham_compile(const char *fp)
{
    struct Lexer *lexer = lexer_alloc(util_read_file(fp));
    struct Token *t = 0;

    while ((t = lexer_get_next_token(lexer))->type != TOKEN_EOF)
    {
        printf("%d %s\n", t->type, t->value);
        token_free(t);
    }

    lexer_free(lexer);
}

