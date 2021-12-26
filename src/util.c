#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>


char *util_read_file(const char *fp)
{
    FILE* file = fopen(fp, "r");

    if (!file)
    {
        fprintf(stderr, "Couldn't open file %s\n", fp);
        return 0;
    }

    char* contents = malloc(sizeof(char));
    contents[0] = '\0';

    char* line = 0;
    size_t len = 0;
    ssize_t read;

    while ((read = getline(&line, &len, file)) != -1)
    {
        int prev_len = strlen(contents);
        contents = realloc(contents, sizeof(char) * (prev_len + read + 1));

        memcpy(&contents[prev_len], line, read);
        contents[prev_len + read] = '\0';
    }

    free(line);
    fclose(file);

    return contents;
}


char *util_int_to_str(int i)
{
    int len;

    if (i == 0)
        len = 1;
    else
        len = (int)((ceil(log10(i)) + 1) * sizeof(char));

    char *str = malloc(sizeof(char) * (len + 1));
    sprintf(str, "%d", i);
    return str;
}

