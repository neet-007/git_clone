#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>
#include <fnmatch.h>
#include "hash_table.h"
#include "repository.h"
#include "utils.h"
#include "git_object_types.h"
#include "git_index.h"
#include "git_object.h"

typedef struct GitIgnore{
    size_t absolute_len;
    GitIgnoreItems **absolute;
    HashTable *scoped;
} GitIgnore;

GitIgnore *git_ignore_read(GitRepository *repo);
int check_ignore(GitIgnore *rules, char *path);
