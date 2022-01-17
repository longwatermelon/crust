#ifndef ARGS_H
#define ARGS_H

#include <stdio.h>
#include <stdbool.h>

struct Args
{
    char *source;

    char *out_filename;

    bool keep_assembly;
};

struct Args *args_parse(int argc, char **argv);
void args_free(struct Args *args);

#endif

