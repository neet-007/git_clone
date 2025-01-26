#ifndef GIT_CLONE_UTILS_H
#define GIT_CLONE_UTILS_H

#define PATH_EXISTS(path) ((path) != NULL && access((path), F_OK) != -1)

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <dirent.h>
#include <sys/stat.h>

char *join_path_(char *path, unsigned int count, va_list va_a);
char *join_path(char *path, unsigned int count, ...);
int make_directories(char *path);
bool is_dir(char *path);
int is_dir_empty(char *path);
int count_digits(int num);

#endif
