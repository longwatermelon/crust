#ifndef UTIL_H
#define UTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

char *util_read_file(const char *fp);
char **util_read_file_lines(const char *fp, size_t *nlines);

char *util_int_to_str(int i);

char *util_strcpy(char *str);
void util_strcat(char **dst, const char *src);

char *util_find_file(char **dirs, size_t ndirs, char *file);
bool util_find_file_dir(char *dir, char *file);

void util_rename_extension(char **file, char *ext);

#endif

