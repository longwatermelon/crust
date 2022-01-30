#ifndef ARGS_H
#define ARGS_H

#include <stdio.h>
#include <stdbool.h>

enum
{
    WARNING_DEAD_CODE,
    WARNING_UNUSED_VARIABLE
};

struct Args
{
    char **sources;
    size_t nsources;

    char *out_filename;

    bool keep_assembly;

    bool warnings[2];

    char **include_dirs;
    size_t include_dirs_len;

    char **libs;
    size_t nlibs;

    char **libdirs;
    size_t nlibdirs;

    bool link_objs;
};

struct Args *args_parse(int argc, char **argv);
void args_free(struct Args *args);

int args_index_from_warning(char *warning, bool *enabled);
char *args_value_from_opt(int argc, char **argv, int *idx);

char *args_advance(int argc, char **argv, int *idx);

#endif

