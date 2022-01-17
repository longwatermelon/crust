#ifndef ARGS_H
#define ARGS_H

#include <stdio.h>

struct Args
{
    char *source;

    char *out_filename;
};

struct Args *args_parse(int argc, char **argv);
void args_free(struct Args *args);

#endif

