#ifndef GIT_CLONE_REFS_H
#define GIT_CLONE_REFS_H

#include "repository.h"

char *ref_resolver(GitRepository *repo, char *ref);
HashTable *ref_list(GitRepository *repo, char *path);

#endif
