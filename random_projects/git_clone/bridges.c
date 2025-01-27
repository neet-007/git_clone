#include "bridges.h"
#include "git_object.h"

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
