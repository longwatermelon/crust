#include "token.h"
#include <stdlib.h>

struct Token *token_alloc(int type, char *value, size_t line_num)
{
    struct Token *t = malloc(sizeof(struct Token));
    t->type = type;
    t->value = value;
    t->line_num = line_num;

    return t;
}


void token_free(struct Token *token)
{
    free(token->value);
    free(token);
}

