#include "args.h"
#include "errors.h"
#include "util.h"

#include <stdlib.h>
#include <string.h>


struct Args *args_parse(int argc, char **argv)
{
    struct Args *args = malloc(sizeof(struct Args));
    args->out_filename = "a.out";
    args->sources = 0;
    args->nsources = 0;
    args->keep_assembly = false;

    args->warnings[WARNING_DEAD_CODE] = true;
    args->warnings[WARNING_UNUSED_VARIABLE] = true;

    args->include_dirs_len = 1;
    args->include_dirs = malloc(sizeof(char*) * args->include_dirs_len);
    args->include_dirs[0] = "lib/include/";

    args->nlibs = 1;
    args->libs = malloc(sizeof(char*) * args->nlibs);
    args->libs[0] = "stdcrust";

    args->nlibdirs = 1;
    args->libdirs = malloc(sizeof(char*) * args->nlibdirs);
    args->libdirs[0] = "lib";

    args->link_objs = true;

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
            else
                args->warnings[idx] = enabled;
        }
        else if (strcmp(argv[i], "--obj") == 0)
        {
            args->link_objs = false;
        }
        else
        {
            args->sources = realloc(args->sources, sizeof(char*) * ++args->nsources);
            args->sources[args->nsources - 1] = argv[i];
        }
    }

    return args;
}


void args_free(struct Args *args)
{
    free(args->sources);
    free(args->include_dirs);
    free(args->libs);
    free(args->libdirs);
    free(args);
}


int args_index_from_warning(char *warning, bool *enabled)
{
    *enabled = true;

    if (strcmp(warning, "dead-code") == 0) return WARNING_DEAD_CODE;
    if (strcmp(warning, "unused-variable") == 0) return WARNING_UNUSED_VARIABLE;

    *enabled = false;

    if (strcmp(warning, "no-dead-code") == 0) return WARNING_DEAD_CODE;
    if (strcmp(warning, "no-unused-variable") == 0) return WARNING_UNUSED_VARIABLE;

    return -1;
}

