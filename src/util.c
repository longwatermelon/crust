#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <dirent.h>


char *util_read_file(const char *fp)
{
    FILE* file = fopen(fp, "r");

    if (!file)
    {
        fprintf(stderr, "Error: Unable to open file '%s'.\n", fp);
        exit(EXIT_FAILURE);
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


char **util_read_file_lines(const char *fp, size_t *nlines)
{
    FILE* file = fopen(fp, "r");

    if (!file)
    {
        fprintf(stderr, "Couldn't open file %s\n", fp);
        return 0;
    }

    char **lines = 0;
    *nlines = 0;

    char* line = 0;
    size_t len = 0;
    ssize_t read;

    while ((read = getline(&line, &len, file)) != -1)
    {
        lines = realloc(lines, sizeof(char*) * ++*nlines);
        lines[*nlines - 1] = malloc(sizeof(char) * (strlen(line) + 1));
        strcpy(lines[*nlines - 1], line);
    }

    free(line);
    fclose(file);

    return lines;
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


char *util_strcpy(char *str)
{
    char *s = malloc(sizeof(char) * (strlen(str) + 1));
    strcpy(s, str);
    return s;
}


void util_strcat(char **dst, char *src)
{
    *dst = realloc(*dst, sizeof(char) * (strlen(*dst) + strlen(src) + 1));
    strcat(*dst, src);
}


char *util_find_file(char **dirs, size_t ndirs, char *file)
{
    FILE *fp = fopen(file, "r");

    if (fp)
    {
        fclose(fp);
        return util_strcpy(file);
    }

    for (size_t i = 0; i < ndirs; ++i)
    {
        if (util_find_file_dir(dirs[i], file))
        {
            char *path = util_strcpy(dirs[i]);
            util_strcat(&path, file);
            return path;
        }
    }

    return 0;
}


bool util_find_file_dir(char *dir, char *file)
{
    DIR *d = opendir(dir);

    if (!d)
        return 0;

    struct dirent *de;

    while ((de = readdir(d)) != 0)
    {
        if (de->d_type == DT_REG)
        {
            if (strcmp(de->d_name, file) == 0)
            {
                closedir(d);
                return true;
            }
        }
    }

    closedir(d);
    return false;
}


void util_rename_extension(char **file, char *ext)
{
    char *f = *file;

    for (size_t i = strlen(f) - 1; i >= 0; --i)
    {
        if (f[i] == '.')
        {
            size_t len = i + strlen(ext);
            char *new = malloc(sizeof(char) * (len + 1));
            f[i] = '\0';
            sprintf(new, "%s%s", f, ext);
            free(f);
            *file = new;
            break;
        }
    }
}

