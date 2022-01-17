#include "args.h"
#include <stdlib.h>
#include <string.h>


struct Args *args_parse(int argc, char **argv)
{
    struct Args *args = malloc(sizeof(struct Args));
    args->out_filename = "a.out";
    args->source = 0;
    args->keep_assembly = false;

    for (int i = 1; i < argc; ++i)
    {
        if (strcmp(argv[i], "-o") == 0)
        {
            args->out_filename = argv[i + 1];
            ++i;
        }
        else if (strcmp(argv[i], "-S") == 0)
        {
            args->keep_assembly = true;
        }
        else
        {
            if (args->source)
            {
                fprintf(stderr, "Error: multiple input files specified\n");
                exit(EXIT_FAILURE);
            }

            args->source = argv[i];
        }
    }

    return args;
}


void args_free(struct Args *args)
{
    free(args);
}

