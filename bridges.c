#include "bridges.h"

int cmd_init(char *path){
    GitRepository *repo = repo_create(path);
    if (repo == NULL){
        fprintf(stderr, "faild to ini repo\n");
        return 1;
    }

    free_repo(repo);
    return 0;
}

// fmt=TYPE_NONE
int cat_file(GitRepository *repo, char *object, GitObjectType fmt){
    char *path = object_find(repo, object, fmt, true);
    if (path == NULL){
        fprintf(stderr, "unable to find object %s path %s\n", object, path);
        return 1;
    }

    GitObject *obj = object_read(repo, path);
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
int cmd_cat_file(char *object, GitObjectType fmt){
    GitRepository *repo = repo_find(".", true);
    if (repo == NULL){
        fprintf(stderr, "repo is null %s %d\n", object, fmt);
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
        free_git_object(commit_object);
        fprintf(stderr, "object is not type commit %s its %d\n", sha, commit_object->type);
        return;
    }

    GitCommit *commit = commit_object->value;
    if (commit->kvlm == NULL){
        free_git_object(commit_object);
        fprintf(stderr, "kvlm is null %s\n", sha);
        return;
    }

    //TODO: see this utf-8 stuff about message and do the \ -> \\, " -> \" replacment
    Ht_item *message = ht_search(commit->kvlm, GIT_CLONE_KVLM_END_KEY);
    if (message == NULL){
        free_git_object(commit_object);
        fprintf(stderr, "end message is not in kvlm %s\n", sha);
        return;
    }
    DynamicArray *message_array = message->value;
    char **message_array_strs = message_array->elements;

    size_t len_message = strlen((char *)(message_array_strs[0]));
    size_t i = 0;
    while(i < len_message){
        if (((char *)(message_array_strs[0]))[i] == '\n'){
            break;
        }
        i++;
    }

    printf("  c_%s [label=\"%.8s: %.*s\"]\n", sha, sha, (int)i, (char *)(char *)message_array_strs[0]);

    Ht_item *parents = ht_search(commit->kvlm, "parent");
    if (parents == NULL){
        free_git_object(commit_object);
        return;
    }

    if (parents->value_type != TYPE_ARRAY){
        free_git_object(commit_object);
        fprintf(stderr, "parent must have type TYPE_ARRAY not %d in log_graphviz\n", parents->value_type);
        return;
    }

    DynamicArray *array = parents->value;
    char **parents_strs = array->elements;
    for (i = 0; i < array->count; i++){
        printf("  c_%s -> c_%s;\n", sha, parents_strs[i]);
        log_graphviz(repo, parents_strs[i], table);
    }
    free_git_object(commit_object);
}

int cmd_log(char *commit){
    GitRepository *repo = repo_find(".", true);
    if (repo == NULL){
        fprintf(stderr, "unable to find repo %s\n", commit);
        return 1;
    }

    printf("digraph git_clone{\n");
    printf("  node[shape=rect]\n");
    char *sha = object_find(repo, commit, TYPE_NONE, true);
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
    free_table(table);
    free(sha);
    return 0;
}

// r=false, prefix=""
int ls_tree(GitRepository *repo, char *ref, bool r, char *prefix){
    char *sha = object_find(repo, ref, TYPE_TREE, true);
    if (sha == NULL){
        fprintf(stderr, "uanble to find sha in ls_tree\n");
        return 1;
    }

    GitObject *object = object_read(repo, sha);
    if (object == NULL){
        fprintf(stderr, "uanble to read object with sha %s in ls_tree\n", sha);
        free(sha);
        return 1;
    }

    if (object->type != TYPE_TREE){
        fprintf(stderr, "object with sha %s is not tree in ls_tree\n", sha);
        free(sha);
        free_git_object(object);
        return 1;
    }

    GitTree *tree = object->value;
    size_t i = 0;
    GitObjectType type;
    GitTreeLeaf *curr = NULL;

    while(i < tree->items_len){
        curr = tree->items[i];
        if (curr->mode[0] == '0'){
            if (curr->mode[1] == '4'){
                type = TYPE_TREE;
            }else{
                fprintf(stderr, "unknown object type in ls_tree %.6s\n", curr->mode);
                free(sha);
                free_git_object(object);
                return 1;
            }
        }else if (curr->mode[0] == '1'){
            if (curr->mode[1] == '0'){
                type = TYPE_BLOB;
            }else if (curr->mode[1] == '2'){
                type = TYPE_BLOB;
            }else if (curr->mode[1] == '6'){
                type = TYPE_COMMIT;
            }else{
                fprintf(stderr, "unknown object type in ls_tree %.6s\n", curr->mode);
                free(sha);
                free_git_object(object);
                return 1;
            }
        }else{
            fprintf(stderr, "unknown object type in ls_tree %.6s\n", curr->mode);
            free(sha);
            free_git_object(object);
            return 1;
        }

        char *joined = join_path(prefix, 1, curr->path);
        if (joined == NULL){
            fprintf(stderr, "unable to join path %s in ls_tree\n", curr->path);
            free(sha);
            free_git_object(object);
            return 1;
        }
        if (!(r && type == TYPE_TREE)){
            printf("%.6s %s %s\t%s\n", curr->mode, "tree", curr->sha, joined);
        }else{
            int res = ls_tree(repo, curr->sha, r, joined);
            if (res != 0){
                fprintf(stderr, "error in ls_tree\n");
                free(joined);
                free(sha);
                free_git_object(object);
                return 1;
            }
        }
        free(joined);
        i++;
    }
    free(sha);
    free_git_object(object);
    return 0;
}

int cmd_ls_tree(char *tree, bool r){
    GitRepository *repo = repo_find(".", true);
    if (repo == NULL){
        fprintf(stderr, "unable to find repo\n");
        return 1;
    }

    int res = ls_tree(repo, tree, r, "");
    free_repo(repo);

    return res;
}

int tree_checkout(GitRepository *repo, GitTree *tree, char *path){
    size_t i = 0;
    GitTreeLeaf *curr = NULL;
    GitObject *curr_object = NULL;
    GitBlob *curr_blob = NULL;
    GitTree *curr_tree = NULL;
    FILE *f = NULL;

    while(i < tree->items_len){
        curr = tree->items[i];

        curr_object = object_read(repo, curr->sha);
        if (curr_object == NULL){
            fprintf(stderr, "unbale to read object with sha %s in tree_checkout\n", curr->sha);
            return 1;
        }

        char *dest = join_path(path, 1, curr->path);
        if (dest == NULL){
            fprintf(stderr, "unbale to read object with sha %s in tree_checkout\n", curr->sha);
            free_git_object(curr_object);
            return 1;
        }

        if (curr_object->type == TYPE_TREE){
            make_directories(dest);
            curr_tree = curr_object->value;
            int res = tree_checkout(repo, curr_tree, dest);
            if (res != 0){
                fprintf(stderr, "error sha %s path %s in tree_checkout\n", curr->sha, dest);
                free(dest);
                free_git_object(curr_object);
                return 1;
            }
        }else if (curr_object->type == TYPE_BLOB){
            //TODO: do symlinks
            curr_blob = curr_object->value;
            f = fopen(dest, "wb");
            if (f == NULL){
                fprintf(stderr, "unbale to open file with path %s in tree_checkout\n", dest);
                free(dest);
                free_git_object(curr_object);
                return 1;
            }

            fwrite(curr_blob->blobdata, sizeof(char), curr_blob->blobdata_size, f);
            fclose(f);
        }
        free_git_object(curr_object);
        free(dest);
        i++;
    }
    return 0;
}

int cmd_checkout(char *commit, char *path){
    GitRepository *repo = repo_find(".", true);
    if (repo == NULL){
        fprintf(stderr, "unable to find repo in cmd_checkout\n");
        return 1;
    }

    char *sha = object_find(repo, commit, TYPE_NONE, true);
    if (sha == NULL){
        fprintf(stderr, "unable to find object commit %s in cmd_checkout\n", commit);
        free_repo(repo);
        return 1;
    }

    GitObject *object = object_read(repo, sha);
    if (object == NULL){
        fprintf(stderr, "unable to raed object commit %s sha %s in cmd_checkout\n", commit, sha);
        free(sha);
        free_repo(repo);
        return 1;
    }

    if (object->type != TYPE_TREE && object->type != TYPE_COMMIT){
        fprintf(stderr, "object must be tree or commit not %d commit %s sha %s in cmd_checkout\n", object->type, commit, sha);
        free(sha);
        free_repo(repo);
        free_git_object(object);
        return 1;
    }

    if (object->type == TYPE_COMMIT){
        GitCommit *commit = object->value;
        Ht_item *tree_item = ht_search(commit->kvlm, "tree");
        if (tree_item == NULL){
            fprintf(stderr, "unable to get tree from commit object %s path c%s in cmd_checkout\n", sha, path);
            free(sha);
            free_repo(repo);
            free_git_object(object);
            return 1;
        }
        if (tree_item->value_type != TYPE_ARRAY){
            fprintf(stderr, "unable to get tree from commit object %s path c%s in cmd_checkout\n", sha, path);
            free(sha);
            free_repo(repo);
            free_git_object(object);
            return 1;
        }
        DynamicArray *tree_array = tree_item->value;
        char **tree_array_elements = tree_array->elements;
        if (tree_array->count <= 0){
            fprintf(stderr, "unable to get tree from commit object %s path c%s in cmd_checkout\n", sha, path);
            free(sha);
            free_repo(repo);
            free_git_object(object);
            return 1;
        }

        GitObject *temp = object_read(repo, tree_array_elements[0]);
        if (temp == NULL){
            fprintf(stderr, "unable to read object sha %s path %s in cmd_checkout\n", sha, path);
            free(sha);
            free_repo(repo);
            free_git_object(object);
            return 1;
        }

        if (temp->type != TYPE_TREE){
            fprintf(stderr, "object must be tree not %d sha %s path %s in cmd_checkout\n", object->type, sha, path);
            free(sha);
            free_repo(repo);
            free_git_object(object);
            free_git_object(temp);
            return 1;
        }
        free_git_object(object);
        object = temp;
    }

    if (PATH_EXISTS(path)){
        if (!is_dir(path)){
            fprintf(stderr, "path does not exitst %s commit %s sha %s in cmd_checkout\n", path, commit, sha);
            free(sha);
            free_repo(repo);
            free_git_object(object);
            return 1;
        }
        if (!is_dir_empty(path)){
            fprintf(stderr, "path is not empty %s commit %s sha %s in cmd_checkout\n", path, commit, sha);
            free(sha);
            free_repo(repo);
            free_git_object(object);
            return 1;
        }
    }else{
        int res = make_directories(path);
        if (res < 1){
            fprintf(stderr, "unable to make dir %s commit %s sha %s in cmd_checkout\n", path, commit, sha);
            free(sha);
            free_repo(repo);
            free_git_object(object);
            return 1;
        }
    }

    char *real_path = realpath(path, NULL);
    if (real_path == NULL){
        fprintf(stderr, "realpath failed for cmd_checkout %s\n", path);
        free(sha);
        free_repo(repo);
        free_git_object(object);
        return NULL;
    }

    int res = tree_checkout(repo, (GitTree *)object->value, real_path);
    free(real_path);
    free(sha);
    free_repo(repo);
    free_git_object(object);

    return res;
}

// with_hash=true, prefix=NULL
int show_ref(GitRepository *repo, HashTable *refs, bool with_hash, char *prefix){
    int res = 0;
    DynamicArray *keys_array = refs->keys;
    Ht_item *curr = NULL;
    char **keys = keys_array->elements;
    bool free_next_prefix = false;
    char *next_prefix = NULL;

    for (size_t i = 0; i < keys_array->count; i++){
        curr = ht_search(refs, keys[i]);
        if (curr == NULL){
            return 1;
        }

        if (curr->value_type == TYPE_STR){
            printf("%s%s%s%s%s\n",with_hash ? (char *)curr->value : "",
                                  with_hash ? " " : "",
                                  prefix ? prefix : "",
                                  prefix ? "/" : "", keys[i]);
        }else if(curr->value_type == TYPE_HASH_TABLE){
            HashTable *h = curr->value;
            if (prefix == NULL){
                free_next_prefix = false;
                next_prefix = keys[i];
            }else{
                free_next_prefix = true;
                next_prefix = malloc(sizeof(char) * (strlen(prefix) + strlen(keys[i]) + 2));
                if (next_prefix == NULL){
                    fprintf(stderr, "unable to allocate memory for next_prefix in show_ref\n");
                    return 1;
                }
                next_prefix[0] = '\0';
                strcpy(next_prefix, prefix);
                next_prefix[strlen(prefix)] = '/';
                next_prefix[strlen(prefix) + 1] = '\0';
                strcpy(next_prefix + strlen(prefix) + 1, keys[i]);
                next_prefix[strlen(prefix) + strlen(keys[i]) + 1] = '\0';
            }
            res = show_ref(repo, h, with_hash, next_prefix);
            if (res != 0){
                if (free_next_prefix && next_prefix != NULL){
                    free(next_prefix);
                }
                fprintf(stderr, "show_ref failed in show_ref\n");
                return 1;
            }
        }
        if (free_next_prefix){
            free_next_prefix = false;
            free(next_prefix);
        }
    }

    return 0;
}

int cmd_show_ref(){
    GitRepository *repo = repo_find(".", true);
    if (repo == NULL){
        fprintf(stderr, "unable to find repo in cmd_show_ref\n");
        return 1;
    }

    HashTable *refs = ref_list(repo, NULL);
    if (refs == NULL){
        fprintf(stderr, "unable to ref list in cmd_show_ref\n");
        free_repo(repo);
        return 1;
    }

    int res = show_ref(repo, refs, true, "refs");
    free_table(refs);
    free_repo(repo);

    return res;
}

int ref_create(GitRepository *repo, char *ref_name, char *sha){
    char *ref_path = malloc(sizeof(char) * (strlen("refs/") + strlen(ref_name) + 1));
    if (ref_path == NULL){
        fprintf(stderr, "unable to allocate memory for ref_path %s %s\n", ref_name, sha);
        return 1;
    }

    ref_path[0] = '\0';
    strcpy(ref_path, "refs/");
    strcpy(ref_path + strlen("refs/"), ref_name);
    char *path = repo_file(repo, false, 1, ref_path);
    free(ref_path);
    if (path == NULL){
        fprintf(stderr, "unable to repo_file for ref_path %s %s\n", ref_name, sha);
        return 1;
    }

    FILE *f = fopen(path, "w");
    if (f == NULL){
        fprintf(stderr, "unable to open file %s\n", path);
    }
    fwrite(sha, sizeof(char), strlen(sha), f);
    fwrite("\n", sizeof(char), 1, f);

    fclose(f);
    free(path);
    return 0;
}

// create_tag_object=true
int tag_create(GitRepository *repo, char *name, char *object, bool create_tag_object){
    int res = 0;
    char *sha = object_find(repo, object, TYPE_NONE, true);
    if (sha == NULL){
        fprintf(stderr, "unable to find object %s in tag_create\n", object);
        return 1;
    }
    char *ref_name = malloc(sizeof(char) * (strlen("tags/") + strlen(name) + 1));
    if (ref_name == NULL){
        fprintf(stderr, "unable to find object %s in tag_create\n", object);
        free(sha);
        return 1;
    }
    ref_name[0] = '\0';
    strcpy(ref_name, "tags/");
    strcpy(ref_name + strlen("tags/"), name);

    if (create_tag_object){
        GitObject *tag = malloc(sizeof(GitObject));
        if (tag == NULL){
            fprintf(stderr, "unable to allocate memory for git object in tag_create\n");
            free(sha);
            return 1;
        }

        tag->type = TYPE_TAG;
        GitTag *tag_ = malloc(sizeof(GitTag));
        if (tag_ == NULL){
            fprintf(stderr, "unable to allocate memory for git object in tag_create\n");
            free(sha);
            free(tag);
            return 1;
        }
        tag_->kvlm = create_table(CAPACITY);
        if (tag_->kvlm == NULL){
            fprintf(stderr, "unable to allocate memory for git object in tag_create\n");
            free(sha);
            free(tag);
            free(tag_);
            return 1;
        }

        DynamicArray *object_a = new_dynamic_array(TYPE_ARRAY_STR, 0);
        add_dynamic_array(object_a, sha);
        ht_insert(tag_->kvlm, "object", object_a, sizeof(DynamicArray), TYPE_ARRAY);

        DynamicArray *type_a = new_dynamic_array(TYPE_ARRAY_STR, 0);
        add_dynamic_array(type_a, "commit");
        ht_insert(tag_->kvlm, "type", type_a, sizeof(DynamicArray), TYPE_ARRAY);

        DynamicArray *tag_a = new_dynamic_array(TYPE_ARRAY_STR, 0);
        add_dynamic_array(tag_a, name);
        ht_insert(tag_->kvlm, "tag", tag_a, sizeof(DynamicArray), TYPE_ARRAY);

        DynamicArray *tagger_a = new_dynamic_array(TYPE_ARRAY_STR, 0);
        add_dynamic_array(tagger_a, "git_clone <git_clone@example.com>");
        ht_insert(tag_->kvlm, "tagger", tagger_a, sizeof(DynamicArray), TYPE_ARRAY);

        DynamicArray *message_a = new_dynamic_array(TYPE_ARRAY_STR, 0);
        add_dynamic_array(message_a, "A tag generated by git_clone, which won't let you customize the message!\n");
        ht_insert(tag_->kvlm, GIT_CLONE_KVLM_END_KEY, message_a, sizeof(DynamicArray), TYPE_ARRAY);

        tag->value = tag_;
        char *tag_sha = object_write(tag, repo);
        if (tag_sha == NULL){
            fprintf(stderr, "unable to write object for git object in tag_create\n");
            free(sha);
            free(tag);
            free(tag_);
            return 1;
        }

        res = ref_create(repo, ref_name, tag_sha);
        free(tag_sha);
        free_git_object(tag);
    }else{
        res = ref_create(repo, ref_name, sha);
    }

    free(sha);
    free(ref_name);
    return res;
}

int cmd_git_tag(char *name, char *object, bool a){
    GitRepository *repo = repo_find(".", true);
    if (repo == NULL){
        fprintf(stderr, "unable to find repo in cmd_git_tag\n");
        return 1;
    }

    if (name != NULL){
        int res = tag_create(repo, name, object, a);
        free_repo(repo);
        return res;
    }

    HashTable *refs = ref_list(repo, NULL);
    Ht_item *h = ht_search(refs, "tags");
    if (h == NULL){
        fprintf(stderr, "unalbe to find key tags in refs in cmd_git_tag\n");
        free_repo(repo);
        free_table(refs);
        return 1;
    }

    if (h->value_type != TYPE_HASH_TABLE){
        fprintf(stderr, "key tags must have type TYPE_HASH_TABLE not %d in refs in cmd_git_tag\n", h->value_type);
        free_repo(repo);
        free_table(refs);
        return 1;
    }

    show_ref(repo, (HashTable *)h->value, false, NULL);

    free_repo(repo);
    free_table(refs);

    return 0;
}

int cmd_rev_parse(char *name, GitObjectType type){
    GitRepository *repo = repo_find(".", true);
    if (repo == NULL){
        fprintf(stderr, "unable to find repo in cmd_rev_parse\n");
        return 1;
    }

    char *res = object_find(repo, name, type, true);
    if (res == NULL){
        free_repo(repo);
        printf("None\n");
        return 1;
    }
    free_repo(repo);

    printf("%s\n", res);
    free(res);
    return 0;
}

int cmd_ls_files(bool verbose){
    GitRepository *repo = repo_find(".", true);
    if (repo == NULL){
        fprintf(stderr, "unable to find repo in cmd_ls_files\n");
        return 1;
    }

    GitIndex *index = read_index(repo);
    if (index == NULL){
        fprintf(stderr, "unable to read index in cmd_ls_files\n");
        free_repo(repo);
        return 1;
    }

    GitIndexEntry *curr;
    char *entry_type = NULL;
    time_t time = 0;
    struct tm *time_info = NULL;
    char formatted_time[120];
    char formatted_time_1[120];
    struct passwd *pw;
    struct group *gr;
    size_t i;
    for (i = 0; i < index->entries_count; i++){
        curr = index->entries[i];
        printf("%s\n", curr->name);
        if (verbose){
            switch (curr->mode_type) {
                case 0b1000:{
                    entry_type = "regular file";
                    break;
                }
                case 0b1010:{
                    entry_type = "sym link";
                    break;
                }
                case 0b1110:{
                    entry_type = "git link";
                    break;
                }
                default:{
                    fprintf(stderr ,"invalid file mode %d\n", curr->mode_type);
                    break;
                }
            }
            printf("  %s with perms: %o\n", entry_type, curr->mode_perms);
            printf("  on blob: %s\n", curr->sha);
            time = curr->ctime_sec;
            time_info = localtime(&time);
            if (time_info == NULL) {
                perror("Failed to convert timestamp");
                continue;
            }
            strftime(formatted_time, sizeof(formatted_time), "%Y-%m-%d %H:%M:%S", time_info);
            snprintf(formatted_time + strlen(formatted_time), sizeof(formatted_time) - strlen(formatted_time), ".%09d", curr->ctime_nsec);
            time = curr->mtime_sec;
            time_info = localtime(&time);
            if (time_info == NULL) {
                perror("Failed to convert timestamp");
                continue;
            }
            strftime(formatted_time_1, sizeof(formatted_time_1), "%Y-%m-%d %H:%M:%S", time_info);
            snprintf(formatted_time_1 + strlen(formatted_time_1), sizeof(formatted_time_1) - strlen(formatted_time_1), ".%09d", curr->mtime_nsec);
            printf("  created: %s, modified: %s\n", formatted_time, formatted_time_1);
            printf("  device: %d, inode: %d\n", curr->dev, curr->ino);
            pw = getpwuid(curr->uid);
            if (!pw) {
                perror("getpwuid failed");
                continue;
            } 
            gr = getgrgid(curr->gid);
            if (!gr) {
                perror("getgrgid failed");
                continue;
            }
            printf("  user: %s %d  group: %s %d\n", pw->pw_name, curr->uid, gr->gr_name, curr->gid);
            printf("  flags: stage= %d assume_valid= %d\n", curr->flag_stage, curr->flag_assume_valid);
        }

        free(curr->name);
        free(curr);
    }
    free(index->entries);
    free(index);

    free_repo(repo);
    return 0;
}

int cmd_check_ignore(size_t count, char **paths){
    GitRepository *repo = repo_find(".", true);
    if (repo == NULL){
        fprintf(stderr, "unable to find repo in cmd_check_ignore\n");
        return 1;
    }

    GitIgnore *rules = git_ignore_read(repo);
    size_t i;
    for (i = 0; i < count; i++){
        if (check_ignore(rules, paths[i]) == 1){
            printf("%s\n", paths[i]);
        }
    }

    size_t j;
    for (i = 0; i < rules->absolute_len; i++){
        for (j = 0; j < rules->absolute[i]->len; j++){
            GitIgnoreItem *curr = rules->absolute[i]->items[j];
            free(curr->path);
            free(curr);
        }
        free(rules->absolute[i]->items);
        free(rules->absolute[i]);
    }

    free(rules->absolute);
    free_table(rules->scoped);
    free(rules);
    free_repo(repo);
    return 0;
}
