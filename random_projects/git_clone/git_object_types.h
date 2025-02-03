#ifndef GIT_CLONE_GIT_OBJECTS_TYPES
#define GIT_CLONE_GIT_OBJECTS_TYPES

#define GIT_TREE_MODE_SIZE 6
#define GIT_SHA_SIZE 41

#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include "dynamic_array.h"
#include "hash_table.h"

typedef enum GitObjectType{
    TYPE_NONE,
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

typedef struct GitTag{
    HashTable *kvlm;
} GitTag;

typedef struct GitIndexEntry {
    int ctime_sec;
    int ctime_nsec;
    int mtime_sec;
    int mtime_nsec;
    int dev;
    int ino;
    int mode;
    int mode_type;
    int mode_perms;
    int uid;
    int gid;
    int fsize;
    char sha[GIT_SHA_SIZE];
    int flags;
    int flag_assume_valid;
    int flag_extended;
    int flag_stage;
    char *name;
} GitIndexEntry;

typedef struct GitIndex {
    uint32_t signature;
    uint32_t version;
    uint32_t entries_count;
    GitIndexEntry **entries;
} GitIndex;

#endif
