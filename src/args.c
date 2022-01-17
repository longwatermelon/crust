#include "args.h"
#include <stdlib.h>
#include <string.h>


struct Args *args_parse(int argc, char **argv)
{
    struct Args *args = malloc(sizeof(struct Args));
    args->out_filename = "a.out";

    for (int i = 0; i < argc; ++i)
    {
        if (strcmp(argv[i], "-o") == 0)
        {
            args->out_filename = argv[i + 1];
            ++i;
        }
        else
        {
            args->source = argv[i];
        }
    }

    return args;
}


void args_free(struct Args *args)
{
    free(args);
}

