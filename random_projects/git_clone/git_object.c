#include "git_object.h"
#include "tree_parser.h"

char *git_object_serialize(GitObject *object, GitRepository *repo, size_t *data_size){
    char *ret = NULL;
    switch (object->type) {
        case TYPE_BLOB:{
            GitBlob *blob = object->value;
            ret = malloc(sizeof(char) * (blob->blobdata_size));
            if (ret == NULL){
                fprintf(stderr, "unable to allocate memory for blobdata in git_object_serializer %ld\n", blob->blobdata_size);
                if (ret){
                    free(ret);
                }
                return NULL;
            }
            *data_size = blob->blobdata_size;
            memcpy(ret, blob->blobdata, sizeof(char) * (blob->blobdata_size));
            break;
        }
        case TYPE_COMMIT:{
            GitCommit *commit = object->value;
            ret = kvlm_serialize(commit->kvlm, data_size);
            if (ret == NULL){
                fprintf(stderr, "unable to serialize kvlm in git_object_serializer\n");
                return NULL;
            }
            break;
        }
        case TYPE_TAG:{

            break;
        }
        case TYPE_TREE:{
            GitTree *tree = object->value;
            ret = tree_serialize(tree, data_size);
            if (ret == NULL){
                fprintf(stderr, "unable to allocate memory for tree_data in git_object_serializer\n");
                return NULL;
            }
            break;
        }
        default:{
            fprintf(stderr, "unknown object type %d\n", object->type);
            if (ret){
                free(ret);
            }
            return NULL;
        }
    }

    return ret;
}

int git_object_deserialize(GitObject *object, void *data, size_t data_size){
    switch (object->type) {
        case TYPE_BLOB:{
            GitBlob *blob = object->value;
            blob->blobdata = malloc(sizeof(char) * data_size);
            if (blob->blobdata == NULL){
                fprintf(stderr, "unable to allocate memory for blobdata in new_git_object\n");
                free(blob);
                return NULL;
            }

            memcpy(blob->blobdata, data, sizeof(char) * data_size);
            blob->blobdata_size = data_size;

            break;
        }
        case TYPE_COMMIT:{
            GitCommit *commit = object->value;
            commit->kvlm = kvlm_parser(data, data_size);
            if (commit->kvlm == NULL){
                fprintf(stderr, "unable to prase kvlm in git_object_desirzile\n");
                return 0;
            }
            break;
        }
        case TYPE_TAG:{

            break;
        }
        case TYPE_TREE:{
            GitTree *tree = object->value;
            tree->items = tree_parse(data, data_size, &tree->items_len, &tree->items_size);
            if (tree->items == NULL){
                if (tree->items == NULL){
                    fprintf(stderr, "unable to prase tree in git_object_desirzile\n");
                    return 0;
                }
            }
            break;
        }
    }

    return 1;
}

GitObject *new_git_object(GitObjectType type, char *data, size_t data_size){
    GitObject *object = malloc(sizeof(GitObject));
    if (object == NULL){
        return NULL;
    }

    switch (type) {
        case TYPE_BLOB:{
            object->type = TYPE_BLOB;
            GitBlob *blob = malloc(sizeof(GitBlob));
            if (blob == NULL){
                fprintf(stderr, "unable to allocate memory for blob in new_git_object\n");
                return NULL;
            }
            object->value = blob;
            break;
        }
        case TYPE_COMMIT:{
            object->type = TYPE_COMMIT;
            GitCommit *commit = malloc(sizeof(GitCommit));
            if (commit == NULL){
                fprintf(stderr, "unable to allocate memory for commit in new_git_object\n");
                return NULL;
            }
            commit->kvlm = NULL;
            object->value = commit;
            break;
        }
        case TYPE_TAG:{

            break;
        }
        case TYPE_TREE:{
            object->type = TYPE_TREE;
            GitTree *tree = malloc(sizeof(GitTree));
            if (tree == NULL){
                fprintf(stderr, "unable to allocate memory for tree in new_git_object\n");
                return NULL;
            }
            tree->items = NULL;
            tree->items_size = 0;
            tree->items_len = 0;
            object->value = tree;
            break;
        }
    }

    if (git_object_deserialize(object, data, data_size) < 1){
        fprintf(stderr, "object deserialization failed\n");
        return NULL;
    }
    return object;
}

//FIXME: fix all warnings better than casting as char *
GitObject *object_read(GitRepository *repo, char *sha){
    char sha_dir[3];
    strncpy(sha_dir, sha, 2);
    sha_dir[2] = '\0';
    char *path = repo_file(repo, false, 3, "objects", sha_dir, sha + 2);
    if (path == NULL){
        fprintf(stderr, "repofile empty\n");
        return NULL;
    }

    if (!is_file(path)){
        fprintf(stderr, "not a file %s\n", path);
        free(path);
        return NULL;
    }

    FILE *f = fopen(path, "rb");
    if (f == NULL){
        fprintf(stderr, "could not open object file in object_read %s\n", path);
        free(path);
        return NULL;
    }

    size_t raw_len = 0;
    unsigned char *raw = decompress_zlib_from_file(f, &raw_len);
    fclose(f);
    if (raw == NULL){
        fprintf(stderr, "could not decompress object file in object_read %s\n", path);
        free(path);
        fclose(f);
        return NULL;
    }

    unsigned int x = 0;
    unsigned int y = 0;

    while(x < raw_len){
        if (raw[x] == ' '){
            break;
        }
        x++;
    }

    unsigned char *fmt = malloc(sizeof(char) * (x + 1));
    if (fmt == NULL){
        fprintf(stderr, "unable to allocate memory for fmt in object_read %s\n", path);
        free(path);
        free(raw);
        return NULL;
    }
    fmt[0] = '\0';
    strncpy((char *)fmt, (char *)raw, x);
    fmt[x] = '\0';

    y = x;
    while (y < raw_len){
        if (raw[y] == '\0'){
            break;
        }
        y++;
    }

    char *size_str = malloc(sizeof(char) * (raw_len - y + 1));
    if (size_str == NULL){
        fprintf(stderr, "unable to allocate memory for size in object_read %s\n", path);
        free(path);
        free(raw);
        return NULL;
    }

    size_str[0] = '\0';
    strncpy(size_str, (char *)(raw + x), raw_len - y);
    size_str[raw_len - y] = '\0';

    size_t size = atoi(size_str);
    free(size_str);

    if (size != raw_len - y - 1){
        fprintf(stderr, "malformed sha in object_read size %ld vs actual %ld path %s\n", size, raw_len - y - 1, path);
        free(path);
        free(raw);
        return NULL;
    }

    GitObjectType type;
    if (strcmp((char *)fmt, "commit") == 0){
        type = TYPE_COMMIT;
    }else if(strcmp((char *)fmt, "tree") == 0){
        type = TYPE_TREE;
    }else if(strcmp((char *)fmt, "tag") == 0){
        type = TYPE_TAG;
    }else if(strcmp((char *)fmt, "blob") == 0){
        type = TYPE_BLOB;
    }else{
        fprintf(stderr, "unkown object type %s in object_read %s\n", fmt, path);
        free(path);
        free(raw);
        free(fmt);
        return NULL;
    }

    GitObject *object = new_git_object(type, (char *)(raw + (y + 1)), size);
    free(fmt);
    free(path);
    free(raw);
    return object;
}

// repo=NULL
char *object_write(GitObject *object, GitRepository *repo){
    size_t data_size = 0;
    char *data = git_object_serialize(object, repo, &data_size);
    if (data == NULL){
        fprintf(stderr, "unable to serialize data for object_write\n");
        return NULL;
    }

    size_t data_size_str_size = count_digits(data_size) + 1;
    char *data_size_str = malloc(sizeof(char) * data_size_str_size);
    if (data_size_str == NULL){
        fprintf(stderr, "unable to allocate memory for data_size_str for object_write\n");
        free(data);
        return NULL;
    }
    snprintf(data_size_str, data_size_str_size, "%ld", data_size);
    data_size_str[data_size_str_size - 1] = '\0';

    size_t to_hash_len = 0;
    char *to_hash = NULL;
    char *fmt = NULL;
    switch (object->type) {
        case TYPE_BLOB:{
            fmt = "blob";
            break;
        }
        case TYPE_COMMIT:{
            fmt = "commit";
            break;
        }
        case TYPE_TAG:{
            fmt = "tag";
            break;
        }
        case TYPE_TREE:{
            fmt = "tree";
            break;
        }
    }
    to_hash_len = strlen(fmt) + 1 + strlen(data_size_str) + 1 + data_size;
    to_hash = malloc(to_hash_len);
    if (to_hash == NULL) {
        fprintf(stderr, "unable to allocate memory for to_hash for object_write\n");
        free(data);
        free(data_size_str);
        return NULL;
    }

    int header_len = snprintf(to_hash, to_hash_len, "%s %s", fmt, data_size_str);
    to_hash[header_len] = '\0';
    memcpy(to_hash + header_len + 1, data, data_size);

    char *hexdigest = malloc(sizeof(char) * (SHA_DIGEST_LENGTH * 2 + 1));
    if (hexdigest == NULL){
        fprintf(stderr, "unable to allocate memory for to_hash for object_write\n");
        free(data);
        free(data_size_str);
        free(to_hash);
        return NULL;
    }

    sha1_hexdigest((unsigned char *)to_hash, to_hash_len, hexdigest);
    if (repo != NULL){
        char *path = repo_file(repo, true, 3, "objects", hexdigest + 2);
        if (path == NULL){
            fprintf(stderr, "unable to allocate memory for to_hash for object_write\n");
            free(data);
            free(data_size_str);
            free(to_hash);
            free(hexdigest);
            return NULL;
        }

        if (!PATH_EXISTS(path)){
            FILE *f = fopen(path, "wb");
            if (f == NULL){
                fprintf(stderr, "unable to allocate memory for to_hash for object_write\n");
                free(data);
                free(data_size_str);
                free(to_hash);
                free(path);
                free(hexdigest);
                return NULL;
            }

            size_t output_size = 0;
            char *compress = compress_zlib(to_hash, to_hash_len, &output_size);
            if (compress == NULL){
                fprintf(stderr, "unable to allocate memory for to_hash for object_write\n");
                free(data);
                free(data_size_str);
                free(to_hash);
                free(path);
                free(hexdigest);
                fclose(f);
                return NULL;
            }

            fwrite(compress, sizeof(char), output_size, f);
            free(path);
            free(compress);
            fclose(f);
        }
    }

    free(data);
    free(to_hash);
    free(data_size_str);
    return hexdigest;
}

void free_git_object(GitObject *object){
    switch (object->type) {
        case TYPE_BLOB:{
            GitBlob *blob = object->value;
            if (blob->blobdata != NULL){
                free(blob->blobdata);
            }
            break;
        }
        case TYPE_COMMIT:{
            GitCommit *commit = object->value;
            if (commit->kvlm != NULL){
                free_table(commit->kvlm);
            }
            break;
        }
        case TYPE_TAG:{
            break;
        }
        case TYPE_TREE:{
            GitTree *tree = object->value;
            size_t i = 0;
            GitTreeLeaf *curr = NULL;
            while(i < tree->items_len){
                curr = tree->items[i];
                free(curr->path);
                free(curr);
                i++;
            }
            free(tree->items);
            break;
        }
    }

    free(object->value);
    free(object);
}

// fmt="", follow=true
char *object_find(GitRepository *repo, char *name, char *fmt, bool follow){
    char *ret = malloc(sizeof(char) * (strlen(name) + 1));
    if (ret == NULL){
        return NULL;
    }

    strcpy(ret, name);
    return ret;
}

// repo=NULL
char *object_hash(FILE *f, char *fmt, GitRepository *repo){
    size_t data_size = 0;
    char *data = file_read_all(f, &data_size);
    if (data == NULL){
        fprintf(stderr, "unable to read file object_hash\n");
        return NULL;
    }

    GitObjectType type;
    GitObject *object = NULL;

    if (strcmp(fmt, "blob") == 0){
        type = TYPE_BLOB;
    }else if (strcmp(fmt, "commmit") == 0){
        type = TYPE_COMMIT;
    }else if (strcmp(fmt, "tag") == 0){
        type = TYPE_TAG;
    }else if (strcmp(fmt, "tree") == 0){
        type = TYPE_TREE;
    }else{
        fprintf(stderr, "unkown object type %s object_hash\n", fmt);
        free(data);
        return NULL;
    }

    object = new_git_object(type, data, data_size);
    if (object == NULL){
        fprintf(stderr, "unable to make object object_hash\n");
        free(data);
        return NULL;
    }

    char *sha = object_write(object, repo);
    free_git_object(object);
    free(data);

    return sha;
}
