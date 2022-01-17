#include "crust.h"
#include "lexer.h"
#include "parser.h"
#include "asm.h"
#include "scope.h"
#include "util.h"
#include <string.h>


void crust_compile(struct Args *args)
{
    struct Lexer *lexer = lexer_alloc(util_read_file(args->source));

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

    struct Asm *as = asm_alloc(args->source);
    asm_gen_expr(as, root);

    FILE *out = fopen("/tmp/a.s", "w");
    fprintf(out, "%s%s\n", as->data, as->root);
    fclose(out);

    system("as --32 /tmp/a.s -o /tmp/a.o");

    const char *template = "ld /tmp/a.o -o %s -m elf_i386";
    char *cmd = calloc(strlen(template) + strlen(args->out_filename) + 1, sizeof(char));
    sprintf(cmd, template, args->out_filename);
    system(cmd);
    free(cmd);

    system("rm /tmp/a{.s,.o}");

    parser_free(parser);
    lexer_free(lexer);
    node_free(root);
    asm_free(as);

    for (size_t i = 0; i < ntokens; ++i)
        token_free(tokens[i]);

    free(tokens);
}

