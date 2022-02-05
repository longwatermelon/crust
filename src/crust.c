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
    char **objs = malloc(sizeof(char*) * args->nsources);
    size_t nobjs = args->nsources;

    for (size_t i = 0; i < args->nsources; ++i)
    {
        crust_compile_file(args, args->sources[i]);

        objs[i] = util_strcpy(args->sources[i]);
        util_rename_extension(&objs[i], ".o");
    }

    if (args->link_objs)
        crust_link(args, objs, nobjs);

    for (size_t i = 0; i < nobjs; ++i)
    {
        if (args->link_objs)
            remove(objs[i]);

        if (!args->keep_assembly)
        {
            util_rename_extension(&objs[i], ".s");
            remove(objs[i]);
        }

        free(objs[i]);
    }

    free(objs);
}


void crust_compile_file(struct Args *args, char *file)
{
    size_t nlines;
    char **source = util_read_file_lines(file, &nlines);
    errors_load_source(source, nlines);

    struct Node *root = crust_gen_ast(args, file);

    // TODO Figure out a better way to find main function
    bool main = false;
    for (size_t i = 0; i < root->compound_size; ++i)
    {
        if (root->compound_nodes[i]->type == NODE_FUNCTION_DEF &&
            strcmp(root->compound_nodes[i]->function_def_name, "main") == 0)
        {
            main = true;
        }
    }

    char *as = crust_gen_asm(root, args, main);
    crust_assemble(as, args, file);

    node_free(root);
    free(as);

    for (size_t i = 0; i < nlines; ++i)
        free(source[i]);

    free(source);
}


struct Node *crust_gen_ast(struct Args *args, char *file)
{
    size_t ntokens;
    struct Token **tokens = crust_tokenize(file, &ntokens);

    struct Parser *parser = parser_alloc(tokens, ntokens, args);
    struct Node *root = parser_parse_compound(parser);

    parser_free(parser);

    for (size_t i = 0; i < ntokens; ++i)
        token_free(tokens[i]);

    free(tokens);

    return root;
}


struct Token **crust_tokenize(char *file, size_t *ntokens)
{
    struct Lexer *lexer = lexer_alloc(util_read_file(file));

    struct Token **tokens = 0;
    *ntokens = 0;
    struct Token *t = 0;

    while ((t = lexer_get_next_token(lexer))->type != TOKEN_EOF)
    {
        tokens = realloc(tokens, sizeof(struct Token*) * ++*ntokens);
        tokens[*ntokens - 1] = t;
    }

    token_free(t);
    lexer_free(lexer);

    return tokens;
}


char *crust_gen_asm(struct Node *root, struct Args *args, bool main)
{
    struct Asm *as = asm_alloc(args, main);
    asm_gen_expr(as, root);

    size_t len = strlen(as->data) + strlen(as->root);
    char *s = malloc(sizeof(char) * (len + 1));
    sprintf(s, "%s%s", as->data, as->root);
    s[len] = '\0';

    asm_free(as);

    return s;
}


void crust_assemble(char *as, struct Args *args, char *file)
{
    char *path = util_strcpy(file);
    util_rename_extension(&path, ".s");

    FILE *out = fopen(path, "w");
    fprintf(out, "%s\n", as);
    fclose(out);

    char *cmd = util_strcpy("as --32 ");

    util_strcat(&cmd, path);
    util_strcat(&cmd, " -o ");

    util_rename_extension(&path, ".o");
    util_strcat(&cmd, path);

    system(cmd);
    free(cmd);
    free(path);
}


void crust_link(struct Args *args, char **files, size_t nfiles)
{
    char *s = util_strcpy("ld -m elf_i386");

    for (size_t i = 0; i < nfiles; ++i)
    {
        util_strcat(&s, " ");
        util_strcat(&s, files[i]);
    }

    for (size_t i = 0; i < args->nlibdirs; ++i)
    {
        util_strcat(&s, " -L ");
        util_strcat(&s, args->libdirs[i]);
    }

    for (size_t i = 0; i < args->nlibs; ++i)
    {
        util_strcat(&s, " -l ");
        util_strcat(&s, args->libs[i]);
    }

    system(s);
    free(s);
}

