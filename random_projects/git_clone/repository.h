#ifndef GIT_CLONE_REPOSITORY_H
#define GIT_CLONE_REPOSITORY_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include "utils.h"
#include "ini_parser.h"

typedef struct GitRepository{
    char *gitdir;
    char *worktree;
    Ini *config;
} GitRepository;

GitRepository *new_git_repository(const char *path, bool force);
char *repo_path(GitRepository *repo, unsigned int count, ...);
char *repo_file(GitRepository *repo, bool mkdir, unsigned int count, ...);
char *repo_dir(GitRepository *repo, bool mkdir, unsigned int count, ...);
char *repo_dir_variadic(GitRepository *repo, bool mkdir, unsigned int count, va_list va);
char *repo_path_variadic(GitRepository *repo, unsigned int count, va_list va);
GitRepository *repo_create(char *path);
void free_repo(GitRepository *repo);

#endif
