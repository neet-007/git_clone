#ifndef GIT_CLONE_GIT_OBJECT
#define GIT_CLONE_GIT_OBJECT

#include <regex.h>
#include "repository.h"
#include "utils.h"
#include "kvlm.h"
#include "tree_parser.h"
#include "git_object_types.h"

char *git_object_serialize(GitObject *object, GitRepository *repo, size_t *data_size);
int git_object_deserialize(GitObject *object, void *data, size_t data_size);
GitObject *new_git_object(GitObjectType type, char *data, size_t data_size);
GitObject *object_read(GitRepository *repo, char *sha);
char *object_write(GitObject *object, GitRepository *repo);
void free_git_object(GitObject *object);
char *object_find(GitRepository *repo, char *name, GitObjectType fmt, bool follow);
char *object_hash(FILE *f, char *fmt, GitRepository *repo);
DynamicArray *object_resolve(GitRepository *repo, char *name);

#endif
