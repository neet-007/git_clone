#ifndef GIT_CLONE_GIT_OBJECT
#define GIT_CLONE_GIT_OBJECT

#include "repository.h"
#include "utils.h"

typedef enum GitObjectType{
    TYPE_BLOB,
    TYPE_COMMIT,
    TYPE_TAG,
    TYPE_TREE,
} GitObjectType;

typedef struct GitObject{
    GitObjectType type;
    void *value;
} GitObject;

typedef struct GitBlob{
    char *blobdata;
    size_t blobdata_size;
} GitBlob;

char *git_object_serialize(GitObject *object, GitRepository *repo, size_t *data_size);
int git_object_deserialize(GitObject *object, void *data);
GitObject *new_git_object(GitObjectType type, char *data, size_t data_size);
GitObject *object_read(GitRepository *repo, char *sha);
char *object_write(GitObject *object, GitRepository *repo);
void free_git_object(GitObject *object);
char *object_find(GitRepository *repo, char *name, char *fmt, bool follow);
char *object_hash(FILE *f, char *fmt, GitRepository *repo);

#endif
