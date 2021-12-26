#include "ham.h"
#include "lexer.h"
#include "parser.h"
#include "util.h"


void ham_compile(const char *fp)
{
    struct Lexer *lexer = lexer_alloc(util_read_file(fp));

    struct Token **tokens = 0;
    size_t ntokens = 0;
    struct Token *t = 0;

    while ((t = lexer_get_next_token(lexer))->type != TOKEN_EOF)
    {
        tokens = realloc(tokens, sizeof(struct Token*) * ++ntokens);
        tokens[ntokens - 1] = t;
    }

    tokens = realloc(tokens, sizeof(struct Token*) * ++ntokens);
    tokens[ntokens - 1] = t;

    struct Parser *parser = parser_alloc(tokens, ntokens);
    struct Node *root = parser_parse(parser);

    parser_free(parser);
    lexer_free(lexer);
    node_free(root);

    for (size_t i = 0; i < ntokens; ++i)
        token_free(tokens[i]);

    free(tokens);
}

