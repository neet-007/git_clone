#ifndef GIT_CLONE_GIT_OBJECTS_TYPES
#define GIT_CLONE_GIT_OBJECTS_TYPES

#define GIT_TREE_MODE_SIZE 6
#define GIT_SHA_SIZE 41

#include "dynamic_array.h"
#include "hash_table.h"
#include <stdlib.h>

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

typedef struct GitCommit{
    HashTable *kvlm;
} GitCommit;

typedef struct GitTreeLeaf{
    char mode[GIT_TREE_MODE_SIZE];
    char sha[GIT_SHA_SIZE];
    char *path;
} GitTreeLeaf;

typedef struct GitTree{
    GitTreeLeaf **items;
    size_t items_size;
    size_t items_len;
} GitTree;

#endif
