#include "ham.h"
#include "util.h"
#include <stdio.h>


int main(int argc, char **argv)
{
    if (argc == 1)
    {
        fprintf(stderr, "No input file provided\n");
        exit(EXIT_FAILURE);
    }

    ham_compile(argv[1]);
    return 0;
}

