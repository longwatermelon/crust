#include "crust.h"
#include "lexer.h"
#include "parser.h"
#include "asm.h"
#include "scope.h"
#include "util.h"


void crust_compile(const char *fp)
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

    token_free(t);

    struct Parser *parser = parser_alloc(tokens, ntokens);
    struct Node *root = parser_parse(parser);

    struct Asm *as = asm_alloc(fp);
    char *s = asm_gen_root(as, root);
    asm_free(as);

    FILE *out = fopen("a.s", "w");
    fprintf(out, "%s\n", s);
    fclose(out);

    system("as --32 a.s -o a.o");
    system("ld a.o -o a.out -m elf_i386");

    free(s);

    parser_free(parser);
    lexer_free(lexer);
    node_free(root);

    for (size_t i = 0; i < ntokens; ++i)
        token_free(tokens[i]);

    free(tokens);
}

