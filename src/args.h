#ifndef ARGS_H
#define ARGS_H

#include <stdio.h>
#include <stdbool.h>

enum
{
    WARNING_DEAD_CODE
};

struct Args
{
    char *source;

    char *out_filename;

    bool keep_assembly;

    bool warnings[1];
};

struct Args *args_parse(int argc, char **argv);
void args_free(struct Args *args);

int args_index_from_warning(char *warning, bool *enabled);

#endif

