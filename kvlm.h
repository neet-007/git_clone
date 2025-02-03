#ifndef GIT_CLONE_KVLM
#define GIT_CLONE_KVLM

#define GIT_CLONE_KVLM_END_KEY "__git_clone_kvlm_end_key__"

#include "hash_table.h"
#include "utils.h"

HashTable *kvlm_parser(char *raw, size_t raw_size);
int kvlm_parse(char *raw, size_t raw_size, HashTable *table, size_t *i, char **key, size_t *key_size, char **value, size_t *value_size);
char *kvlm_serialize(HashTable *kvlm, size_t *data_size);

#endif
