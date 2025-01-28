#include "bridges.h"
#include "git_object.h"
#include "hash_table.h"

int cmd_init(char *path){
    GitRepository *repo = repo_create(path);
    if (repo == NULL){
        fprintf(stderr, "faild to ini repo\n");
        return 1;
    }

    free_repo(repo);
    return 0;
}

// fmt=""
int cat_file(GitRepository *repo, char *object, char *fmt){
    char *path = object_find(repo, object, fmt, true);
    if (path == NULL){
        fprintf(stderr, "unable to find object %s path %s\n", object, path);
        return 1;
    }

    GitObject *obj = object_read(repo, object);
    if (obj == NULL){
        fprintf(stderr, "unable to read object %s path %s\n", object, path);
        free(path);
        return 1;
    }

    size_t data_size = 0;
    char *data = git_object_serialize(obj, repo, &data_size);
    if (data == NULL){
        fprintf(stderr, "unable to serialize object %s path %s\n", object, path);
        free_git_object(obj);
        free(data);
        free(path);
        return 1;
    }

    print_raw_data_as_chars(data, data_size);
    free_git_object(obj);
    free(data);
    free(path);
    return 0;
}

// fmt=""
int cmd_cat_file(char *object, char *fmt){
    GitRepository *repo = repo_find(".", true);
    if (repo == NULL){
        fprintf(stderr, "repo is null %s %s\n", object, fmt);
        return 1;
    }

    int res = cat_file(repo, object, fmt);
    free_repo(repo);
    return res;
}

int cmd_hash_object(char *path, char *type, bool write){
    GitRepository *repo = NULL;

    if (write){
        repo = repo_find(".", true);
    }

    FILE *f = fopen(path, "rb");
    if (f == NULL){
        if (repo != NULL){
            free(repo);
        }
        fprintf(stderr, "unable to open file %s\n", path);
        return 1;
    }
    char *sha = object_hash(f, type, repo);
    printf("%s\n", sha);

    if (repo != NULL){
        free(repo);
    }
    fclose(f);
    free(sha);
    return 0;
}

void log_graphviz(GitRepository *repo, char *sha, HashTable *table){
    Ht_item *res = ht_search(table, sha);
    if (res != NULL){
        return;
    }

    bool dummy = false;
    ht_insert(table, sha, &dummy, sizeof(bool), TYPE_BOOL);

    GitObject *commit_object = object_read(repo, sha);
    if (commit_object == NULL){
        fprintf(stderr, "unable to read object %s\n", sha);
        return;
    }
    if (commit_object->type != TYPE_COMMIT){
        fprintf(stderr, "object is not type commit %s its %d\n", sha, commit_object->type);
        return;
    }

    GitCommit *commit = commit_object->value;
    if (commit->kvlm == NULL){
        fprintf(stderr, "kvlm is null %s\n", sha);
        return;
    }

    //TODO: see this utf-8 stuff about message and do the \ -> \\, " -> \" replacment
    Ht_item *message = ht_search(commit->kvlm, GIT_CLONE_KVLM_END_KEY);
    if (message == NULL){
        fprintf(stderr, "end message is not in kvlm %s\n", sha);
        return;
    }
    DynamicArray *message_array = message->value;
    char **message_array_strs = message_array->elements;

    size_t len_message = strlen((char *)message_array_strs[0]);
    size_t i = 0;
    while(i < len_message){
        if (((char *)message->value)[i] == '\n'){
            break;
        }
        i++;
    }

    printf("  c_%s [label=\"%.8s: %.*s\"]\n", sha, sha, (int)i, (char *)(char *)message_array_strs[0]);

    Ht_item *parents = ht_search(commit->kvlm, "parent");
    if (parents == NULL){
        return;
    }

    if (parents->value_type != TYPE_ARRAY){
        fprintf(stderr, "parent must have type TYPE_ARRAY not %d in log_graphviz\n", parents->value_type);
        return;
    }

    DynamicArray *array = parents->value;
    char **parents_strs = array->elements;
    for (i = 0; i < array->count; i++){
        printf("  c_%s -> c_%s;\n", sha, parents_strs[i]);
        log_graphviz(repo, parents_strs[i], table);
    }
}

int cmd_log(char *commit){
    GitRepository *repo = repo_find(".", true);
    if (repo == NULL){
        fprintf(stderr, "unable to find repo %s\n", commit);
        return 1;
    }

    printf("digraph git_clone{\n");
    printf("  node[shape=rect]\n");
    char *sha = object_find(repo, commit, "", true);
    if (sha == NULL){
        fprintf(stderr, "unable to find commit %s\n", commit);
        free_repo(repo);
        return 1;
    }

    HashTable *table = create_table(CAPACITY);
    if (table == NULL){
        fprintf(stderr, "unable to craete table in cmd_log %s %s\n", commit, sha);
        free(sha);
        free_repo(repo);
        return 1;
    }

    log_graphviz(repo, sha, table);
    printf("}\n");

    free_repo(repo);
    free(sha);
    return 0;
}
