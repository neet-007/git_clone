#include "kvlm.h"

HashTable *kvlm_parser(char *raw, size_t raw_size){
    size_t key_size = 128;
    size_t val_size = 128;
    size_t i = 0;
    int res = 1;

    HashTable *table = create_table(CAPACITY);
    if (table == NULL){
        fprintf(stderr, "unable to allocate table in kvlm_parser\n");
        return NULL;
    }
    char *key = malloc(sizeof(char) * key_size);
    if (key == NULL){
        fprintf(stderr, "unable to allocate memory for key in kvlm_parser\n");
        free_table(table);
        return NULL;
    }
    char *val = malloc(sizeof(char) * val_size);
    if (val == NULL){
        fprintf(stderr, "unable to allocate memory for key in kvlm_parser\n");
        free(key);
        free(val);
        free_table(table);
        return NULL;
    }

    while(res){
        res = kvlm_parse(raw, raw_size, table, &i, &key, &key_size, &val, &val_size);
        if (res == -1){
            fprintf(stderr, "kvlm parsing fialed\n");
            if (table != NULL){
                free_table(table);
            }
            if (key != NULL){
                free(key);
            }
            if (val != NULL){
                free(val);
            }
            return NULL;
        }
    }

    if (key != NULL){
        free(key);
    }
    if (val != NULL){
        free(val);
    }

    return table;
}

int kvlm_parse(char *raw, size_t raw_size, HashTable *table, size_t *i, char **key, size_t *key_size, char **value, size_t *value_size){
    int spc = find_char(raw, ' ', *i, raw_size);
    int nl = find_char(raw, '\n', *i, raw_size);

    if (spc < 0 || nl < spc){
        if (nl != *i){
            fprintf(stderr, "nl must be the same as curr raw index %d vs %ld\n", nl, *i);
            return -1;
        }

        if (strlen(GIT_CLONE_KVLM_END_KEY) >= *key_size){
            (*key_size) = (strlen(GIT_CLONE_KVLM_END_KEY)) * 2;
            char *temp = realloc(*key, sizeof(char) * (*key_size));
            if (temp == NULL){
                fprintf(stderr, "1unable to allocate memory for key in kvlm_parse\n");
                return -1;
            }
            *key = temp;
        }

        if (raw_size - (*i + 1) >= *value_size){
            (*value_size) = (raw_size - (*i + 1)) * 2;
            char *temp = realloc(*value, sizeof(char) * (*value_size));
            if (temp == NULL){
                value = NULL;
                fprintf(stderr, "unable to allocate memory for value in kvlm_parse\n");
                return -1;
            }
            *value = temp;
        }

        memcpy(*key, GIT_CLONE_KVLM_END_KEY, sizeof(char) * (strlen(GIT_CLONE_KVLM_END_KEY)));
        memcpy(*value, raw + (*i + 1), sizeof(char) * (raw_size - (*i + 1)));
        (*key)[strlen(GIT_CLONE_KVLM_END_KEY)] = '\0';
        (*value)[raw_size - (*i + 1)] = '\0';
        Ht_item *item = ht_search(table, *key);
        if (item == NULL){
            DynamicArray *array = new_dynamic_array(TYPE_ARRAY_STR, 0);
            add_dynamic_array(array, *value);
            ht_insert(table, *key, array, sizeof(DynamicArray), TYPE_ARRAY);
            free(array);
        }else{
            DynamicArray *array = item->value;
            add_dynamic_array(array, *value);
        }

        return 0;
    }

    if (spc - (*i) >= *key_size){
        (*key_size) = (spc - (*i)) * 2;
        char *temp = realloc(*key, sizeof(char) * (*key_size));
        if (temp == NULL){
            fprintf(stderr, "2unable to allocate memory for key in kvlm_parse %d %ld\n", spc, spc - (*i));
            return -1;
        }
        *key = temp;
    }

    int end = *i;
    while(1){
        end = find_char(raw, '\n', end+1, raw_size);
        if (end >= raw_size){
            break;
        }
        if (raw[end + 1] != ' '){
            break;
        }
    }

    if (end - (spc + 1) >= *value_size){
        (*value_size) = (end - (spc + 1)) * 2;
        char *temp = realloc(*value, sizeof(char) * (*value_size));
        if (temp == NULL){
            fprintf(stderr, "unable to allocate memory for value in kvlm_parse\n");
            return -1;
        }
        *value = temp;
    }

    memcpy(*key, raw + (*i), sizeof(char) * (spc - (*i)));
    memcpy(*value, raw + (spc + 1), sizeof(char) * (end - (spc + 1)));
    (*key)[spc - (*i)] = '\0';
    (*value)[end - (spc + 1)] = '\0';

    Ht_item *item = ht_search(table, *key);
    if (item == NULL){
        DynamicArray *array = new_dynamic_array(TYPE_ARRAY_STR, 0);
        add_dynamic_array(array, *value);
        ht_insert(table, *key, array, sizeof(DynamicArray), TYPE_ARRAY);
        free(array);
    }else{
        DynamicArray *array = item->value;
        add_dynamic_array(array, *value);
    }

    *i = end + 1;
    return 1;
}

char *kvlm_serialize(HashTable *kvlm, size_t *data_size){
    size_t size = 0;
    size_t curr = 0;
    size_t i;
    char *ret = NULL;

    char **kvlm_keys = kvlm->keys->elements;
    Ht_item *val = NULL;
    DynamicArray *val_array = NULL;
    char **val_str_array = NULL;
    for (size_t i = 0; i < kvlm->keys->count; i++){
        val = ht_search(kvlm, kvlm_keys[i]);
        if (val == NULL){
            fprintf(stderr, "misssing key in kvlm %s\n", kvlm_keys[i]);
            continue;
        }

        if (val->value_type != TYPE_ARRAY){
            fprintf(stderr, "kvlm value must have type TYPE_ARRAY not %d\n", val->value_type);
            if (ret != NULL){
                free(ret);
            }
            return NULL;
        }

        val_array = val->value;
        val_str_array = val_array->elements;

        for (i = 0; i < val_array->count; i++){
            size += (strlen(val_str_array[i]) + 1);
        }
    }
    size++;
    ret = malloc(sizeof(char) * size);
    if (ret == NULL){
        fprintf(stderr, "unable to allocate memory for ret\n");
        return NULL;
    }

    for (size_t i = 0; i < kvlm->keys->count; i++){
        if (strcmp(kvlm_keys[i], GIT_CLONE_KVLM_END_KEY) == 0){
            continue;
        }
        val = ht_search(kvlm, kvlm_keys[i]);
        if (val == NULL){
            fprintf(stderr, "misssing key in kvlm %s\n", kvlm_keys[i]);
            continue;
        }

        if (val->value_type != TYPE_ARRAY){
            fprintf(stderr, "kvlm value must have type TYPE_ARRAY not %d\n", val->value_type);
            if (ret != NULL){
                free(ret);
            }
            return NULL;
        }

        val_array = val->value;
        val_str_array = val_array->elements;

        for (i = 0; i < val_array->count; i++){
            memcpy(ret + curr, val_str_array[i], strlen(val_str_array[i]));
            curr += strlen(val_str_array[i]);
            ret[curr + 1] = '\n';
        }
    }

    val = ht_search(kvlm, GIT_CLONE_KVLM_END_KEY);
    if (val == NULL){
        fprintf(stderr, "misssing end message key in kvlm %s\n", kvlm_keys[i]);
        if (ret != NULL){
            free(ret);
        }
        return NULL;
    }

    val_array = val->value;
    val_str_array = val_array->elements;
    ret[curr++] = '\n';
    memcpy(ret + curr, val_str_array[0], strlen(val_str_array[0]));
    
    return ret;
}
