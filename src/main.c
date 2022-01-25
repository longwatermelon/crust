#include "crust.h"

#include <stdio.h>
#include <stdlib.h>


int main(int argc, char **argv)
{
    if (argc == 1)
    {
        fprintf(stderr, "No input file provided\n");
        exit(EXIT_FAILURE);
    }

    struct Args *args = args_parse(argc, argv);
    crust_compile(args);
    args_free(args);

    return 0;
}

