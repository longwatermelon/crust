#include "crust.h"
#include "lexer.h"
#include "parser.h"
#include "asm.h"
#include "scope.h"
#include "util.h"
#include "errors.h"

#include <string.h>


void crust_compile(struct Args *args)
{
    size_t nlines;
    char **source = util_read_file_lines(args->source, &nlines);
    errors_load_source(source, nlines);

    struct Node *root = crust_gen_ast(args);
    char *as = crust_gen_asm(root, args);
    crust_assemble(as, args);

    node_free(root);
    free(as);

    for (size_t i = 0; i < nlines; ++i)
        free(source[i]);

    free(source);
}


struct Node *crust_gen_ast(struct Args *args)
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

    parser_free(parser);
    lexer_free(lexer);

    for (size_t i = 0; i < ntokens; ++i)
        token_free(tokens[i]);

    free(tokens);

    return root;
}


char *crust_gen_asm(struct Node *root, struct Args *args)
{
    struct Asm *as = asm_alloc(args);
    asm_gen_expr(as, root);

    size_t len = strlen(as->data) + strlen(as->root);
    char *s = malloc(sizeof(char) * (len + 1));
    sprintf(s, "%s%s", as->data, as->root);
    s[len] = '\0';

    asm_free(as);

    return s;
}


void crust_assemble(char *as, struct Args *args)
{
    FILE *out = fopen("/tmp/a.s", "w");
    fprintf(out, "%s\n", as);
    fclose(out);

    system("as --32 /tmp/a.s -o /tmp/a.o");

    const char *template = "ld /tmp/a.o -o %s -m elf_i386";
    char *cmd = calloc(strlen(template) + strlen(args->out_filename) + 1, sizeof(char));
    sprintf(cmd, template, args->out_filename);
    system(cmd);
    free(cmd);

    if (args->keep_assembly)
        system("cp /tmp/a.s .");

    remove("/tmp/a.s");
    remove("/tmp/a.o");
}

