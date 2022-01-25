#include "args.h"
#include "errors.h"

#include <stdlib.h>
#include <string.h>


struct Args *args_parse(int argc, char **argv)
{
    struct Args *args = malloc(sizeof(struct Args));
    args->out_filename = "a.out";
    args->source = 0;
    args->keep_assembly = false;

    args->warnings[WARNING_DEAD_CODE] = true;

    for (int i = 1; i < argc; ++i)
    {
        if (strcmp(argv[i], "--help") == 0)
        {
            printf( "Crust command line help\n"
                    "-o [output file]: Specify output executable name\n"
                    "-S: Keep assembly output\n");
            exit(0);
        }
        else if (strcmp(argv[i], "-o") == 0)
        {
            args->out_filename = argv[i + 1];
            ++i;
        }
        else if (strcmp(argv[i], "-S") == 0)
        {
            args->keep_assembly = true;
        }
        else if (strncmp(argv[i], "-W", 2) == 0)
        {
            char *warning = &argv[i][2];

            bool enabled;
            int idx = args_index_from_warning(warning, &enabled);

            if (idx == -1)
                errors_args_nonexistent_warning(warning);

            args->warnings[idx] = enabled;
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


int args_index_from_warning(char *warning, bool *enabled)
{
    if (strcmp(warning, "no-dead-code") == 0) return WARNING_DEAD_CODE;

    return -1;
}

