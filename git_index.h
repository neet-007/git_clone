#ifndef GIT_CLONE_GIT_INDEX
#define GIT_CLONE_GIT_INDEX

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "git_object_types.h"
#include "repository.h"

void read_git_index(char *filename);
GitIndex *read_index(GitRepository *repo);

#endif
