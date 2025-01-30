#ifndef GIT_CLONE_TREE_PARSER
#define GIT_CLONE_TREE_PARSER

#include <stdlib.h>
#include "git_object_types.h"
#include "utils.h"

GitTreeLeaf **tree_parse(char *raw, size_t raw_size, size_t *leafs_len, size_t *leafs_size);
char *tree_serialize(GitTree *tree, size_t *data_size);

#endif
