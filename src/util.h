#ifndef UTIL_H
#define UTIL_H

#include <stdio.h>
#include <stdlib.h>

char *util_read_file(const char *fp);
char **util_read_file_lines(const char *fp, size_t *nlines);

char *util_int_to_str(int i);

char *util_strcpy(char *str);

#endif

