#include "git_ignore.h"
#include "utils.h"

GitIgnoreItem *git_ignore_parse_line(char *path){
    size_t path_len = strlen(path);
    int i = path_len;
    char *new_path = path;

    while(i >= 0){
        if (isspace(new_path[i])){
            new_path[i] = '\0';
            i--;
        }else{
            break;
        }
    }

    i = 0;
    while(i < path_len){
        if (isspace(new_path[0])){
            new_path++;
            i++;
        }else{
            break;
        }
    }

    GitIgnoreItem *item = malloc(sizeof(GitIgnoreItem));
    if (strlen(new_path) == 0 && new_path[0] == '#'){
        return NULL;
    }
    if (new_path[0] == '!'){
        item->included = false;
        item->path= strdup(new_path + 1);
        return item;
    }
    if (new_path[0] == '\\'){
        item->included = true;
        item->path = strdup(new_path + 1);
        return item;
    }

    item->included = true;
    item->path = strdup(new_path);

    return item;
}

GitIgnoreItem **git_ignore_parse_lines(char ** lines, size_t lines_count, size_t *parsed_lines_count){
    size_t size = lines_count;
    GitIgnoreItem **ret = malloc(sizeof(GitIgnoreItem *) * lines_count);
    if (ret == NULL){
        fprintf(stderr, "unable to allocate memory for git_ignore_parse_lines\n");
        return NULL;
    }

    size_t i;
    GitIgnoreItem *curr = NULL;
    for (i = 0; i < lines_count; i++){
        curr = git_ignore_parse_line(lines[i]);
        if (curr != NULL){
            ret[(*parsed_lines_count)++] = curr;
        }
    }

    return ret;
}

GitIgnore *git_ignore_read(GitRepository *repo){
    GitIgnore *ret = malloc(sizeof(GitIgnore));
    if (ret == NULL){
        fprintf(stderr, "unable to allocate memory for git ignore in git_ignore_read\n");
        return NULL;
    }

    char *excluded_path = repo_file(repo, false, 1, "info/exclude");
    if (excluded_path == NULL){
        fprintf(stderr, "unable to get repo path for info/exculde in git_ignore_read\n");
        free(ret);
        return NULL;
    }

    FILE *f = NULL;
    char *temp = NULL;
    char buf[1024];

    GitIgnoreItem **temp_arr = NULL;
    size_t abs_size = 8;
    size_t abs_len = 0;
    GitIgnoreItems **abs = malloc(sizeof(GitIgnoreItems *) * abs_size);
    if (abs == NULL){
        fprintf(stderr, "unable to allocate memory for abs array in git_ignore_read\n");
        free(excluded_path);
        free(ret);
        return NULL;
    }

    if (PATH_EXISTS(excluded_path)){
        GitIgnoreItems *excluded = malloc(sizeof(GitIgnoreItems));
        excluded->size = 8;
        excluded->len = 0;
        excluded->items = malloc(sizeof(GitIgnoreItem *) * excluded->size);
        if (excluded->items == NULL){
            fprintf(stderr, "unable to allocate memory for abs array in git_ignore_read\n");
            free(excluded_path);
            free(ret);
            free(abs);
            return NULL;
        }
        f = fopen(excluded_path, "r");
        if (f == NULL){
            fprintf(stderr, "unable to open file %s in git_ignore_read\n", excluded_path);
            free(abs);
            free(excluded_path);
            free(ret);
            return NULL;
        }

        while(fgets(buf, sizeof(buf), f) != NULL){
            GitIgnoreItem *temp = git_ignore_parse_line(buf);
            if (temp->included){
                if (excluded->len>= excluded->size){
                    excluded->size *= 2;
                    temp_arr = realloc(excluded, sizeof(GitIgnoreItem *) * excluded->size);
                    if (temp_arr == NULL){
                        fprintf(stderr, "unable to allocate memory for abs arr %s in git_ignore_read\n", excluded_path);
                        free(abs);
                        free(excluded_path);
                        free(ret);
                        return NULL;
                    }
                    excluded->items = temp_arr;
                }
                excluded->items[excluded->len] = temp;
                if (excluded->items[excluded->len] == NULL){
                    fprintf(stderr, "unable to in path %s in git_ignore_read\n", excluded_path);
                    free(abs);
                    free(excluded_path);
                    free(ret);
                    return NULL;
                }
                excluded->len++;
            }
        } 
        if (abs_len>= abs_size){
            abs_size *= 2;
            GitIgnoreItems **temp_abs= realloc(abs, sizeof(GitIgnoreItems *) * abs_size);
            if (temp_arr == NULL){
                fprintf(stderr, "unable to allocate memory for abs arr %s in git_ignore_read\n", "a");
                free(abs);
                free(excluded_path);
                free(ret);
                return NULL;
            }
            abs= temp_abs;
        }
        abs[abs_len++] = excluded;
    }

    free(excluded_path);
    char *config_home = getenv("XDG_CONFIG_HOME");

    if (config_home == NULL) {
        struct passwd *pw = getpwuid(getuid());
        if (pw == NULL) {
            perror("unable to get home directory in git_ignore_read\n");
            free(abs);
            free(ret);
            return NULL;
        }
        config_home = malloc(strlen(pw->pw_dir) + strlen("/.config") + 1);
        if (config_home == NULL){
            fprintf(stderr, "unable to allocate memory for config home in git_ignore_read\n");
            free(abs);
            free(ret);
            //free(pw);
            return NULL;
        }
        strcpy(config_home, pw->pw_dir);
        strcat(config_home, "/.config");
        //free(pw);
    }

    size_t globale_file_size = strlen(config_home) + strlen("/git/ignore");
    char *global_file = malloc(sizeof(char) * (globale_file_size + 1));
    if (global_file == NULL){
        fprintf(stderr, "unable to allocate memory for global_file in git_ignore_read\n");
        free(abs);
        free(ret);
    }
    snprintf(global_file, sizeof(char) * globale_file_size, "%s/git/ignore", config_home);

    if (PATH_EXISTS(global_file)){
        GitIgnoreItems *global = malloc(sizeof(GitIgnoreItems));
        global->size = 8;
        global->len = 0;
        global->items = malloc(sizeof(GitIgnoreItem *) * global->size);
        if (global->items == NULL){
            fprintf(stderr, "unable to allocate memory for abs array in git_ignore_read\n");
            free(ret);
            free(global_file);
            free(abs);
            return NULL;
        }
        f = fopen(global_file, "r");
        if (f == NULL){
            fprintf(stderr, "unable to open file %s in git_ignore_read\n", global_file);
            free(abs);
            free(global_file);
            free(ret);
            return NULL;
        }

        while(fgets(buf, sizeof(buf), f) != NULL){
            GitIgnoreItem *temp = git_ignore_parse_line(buf);
            if (temp->included){
                if (global->len >= global->size){
                    global->size *= 2;
                    temp_arr = realloc(abs, sizeof(GitIgnoreItems *) * global->size);
                    if (temp_arr == NULL){
                        fprintf(stderr, "unable to allocate memory for abs arr %s in git_ignore_read\n", global_file);
                        free(abs);
                        free(global_file);
                        free(ret);
                        return NULL;
                    }
                    global->items = temp_arr;
                }
                global->items[global->len] = temp;
                if (global->items[global->len] == NULL){
                    fprintf(stderr, "unable to in path %s in git_ignore_read\n", global_file);
                    free(abs);
                    free(global_file);
                    free(ret);
                    return NULL;
                }
                global->len++;
            }
        } 
        if (abs_len>= abs_size){
            abs_size *= 2;
            GitIgnoreItems **temp_abs= realloc(abs, sizeof(GitIgnoreItems *) * abs_size);
            if (temp_arr == NULL){
                fprintf(stderr, "unable to allocate memory for abs arr %s in git_ignore_read\n", "a");
                free(abs);
                free(ret);
                free(global_file);
                return NULL;
            }
            abs= temp_abs;
        }
        abs[abs_len++] = global;
    }

    free(global_file);
    if (config_home != getenv("XDG_CONFIG_HOME")) {
        free(config_home);
    }

    GitIndex *index = read_index(repo);
    if (index == NULL){
        fprintf(stderr, "unable to read index in git_ignore_read\n");
        free(abs);
        free(ret);
        return NULL;
    }
    ret->absolute = abs;
    ret->absolute_len = abs_len;

    char *raw_line = NULL;
    size_t raw_line_size = 0;
    size_t start = 0;

    size_t entry_index;
    char *entry_name = NULL;
    GitObject *curr_object = NULL;
    GitBlob *curr_blob = NULL;
    GitIndexEntry *curr_entry = NULL;
    HashTable *scoped = create_table(CAPACITY);

    for (entry_index = 0; entry_index < index->entries_count; entry_index++){
        GitIgnoreItems *curr = malloc(sizeof(GitIgnoreItems));
        if (curr == NULL){
            fprintf(stderr, "unable to allocate memory for curr array in git_ignore_read\n");
            free(ret);
            if (raw_line != NULL){
                free(raw_line);
            }
            return NULL;
        }
        curr->size = 8;
        curr->len = 0;
        curr->items = malloc(sizeof(GitIgnoreItem *) * curr->size);

        curr_entry = index->entries[entry_index];
        entry_name = curr_entry->name;
        if (strcmp(entry_name, ".gitignore") == 0 || ends_with(entry_name, "/.gitignore")){
            dirname(&entry_name);
            curr_object = object_read(repo, curr_entry->sha);
            if (curr_object == NULL){
                fprintf(stderr, "unable to read object %s in git_ignore_read\n", entry_name);
                free(abs);
                free(ret);
                if (raw_line != NULL){
                    free(raw_line);
                }
                return NULL;
            }
            if (curr_object->type != TYPE_BLOB){
                fprintf(stderr, "object type must be TYPE_BLOB not %d %s in git_ignore_read\n",curr_object->type, entry_name);
                free(abs);
                free(ret);
                free(curr_object);
                if (raw_line != NULL){
                    free(raw_line);
                }
                return NULL;
            }

            curr_blob = curr_object->value;
            while(read_line_from_raw(&raw_line, curr_blob->blobdata, &raw_line_size, &start, curr_blob->blobdata_size) != NULL){
                GitIgnoreItem *temp = git_ignore_parse_line(raw_line);
                if (temp->included){
                    if (curr->len>= curr->size){
                        curr->size *= 2;
                        temp_arr = realloc(curr, sizeof(GitIgnoreItems *) * curr->size);
                        if (temp_arr == NULL){
                            fprintf(stderr, "unable to allocate memory for curr arr %s in git_ignore_read\n", excluded_path);
                            free(curr);
                            free(ret);
                            free(curr_object);
                        if (raw_line != NULL){
                            free(raw_line);
                        }
                            return NULL;
                        }
                        curr->items = temp_arr;
                    }
                    curr->items[curr->len] = temp;
                    if (curr->items[curr->len] == NULL){
                        fprintf(stderr, "unable to in path %s in git_ignore_read\n", excluded_path);
                        free(curr);
                        free(ret);
                        free(curr_object);
                        if (raw_line != NULL){
                            free(raw_line);
                        }
                        return NULL;
                    }
                    curr->len++;
                }
            }
            free_git_object(curr_object);
            ht_insert(scoped, entry_name, curr, sizeof(GitIgnoreItems), TYPE_GIT_IGNORE_ITEM);
        }
        free(curr);
    }

    if (raw_line != NULL){
        free(raw_line);
    }
    
    for (size_t j = 0; j < index->entries_count; j++){
        free(index->entries[j]->name);
        free(index->entries[j]);
    }
    free(index->entries);
    free(index);

    ret->scoped = scoped;
    return ret;
}

int check_ignore1(GitIgnoreItems *rules, char *path){
    int ret = -1;
    GitIgnoreItem *item = NULL;
    size_t i;

    for (i = 0; i < rules->len; i++){
        item = rules->items[i];
        if (fnmatch(path, item->path, 0) == 0){
            ret = item->included;
        }
    }

    return ret;
}

int check_ignore_scoped(HashTable *rules, char *path){
    char *temp = NULL;
    char *parent = dirname_copy(path);
    Ht_item *item = NULL;
    int res = -1;
    while(1){
        item = ht_search(rules, parent);
        if (item != NULL){
            res = check_ignore1((GitIgnoreItems *)item->value, path);
            if (res != -1){
                free(parent);
                return res;
            }
        }

        if (strcmp(parent, ".") == 0){
            free(parent);
            return -1;
        }

        temp = dirname_copy(parent);
        free(parent);
        parent = temp;
    }
}

int check_ignore_absolute(GitIgnoreItems **rules, size_t count, char *path){
    GitIgnoreItems *list = NULL;
    int res = -1;

    size_t i;
    for (i = 0; i < count; i++){
        list = rules[i];
        res = check_ignore1(list, path);
        if (res != -1){
            return res;
        }
    }

    return 0;
}

int check_ignore(GitIgnore *rules, char *path){
    if (strlen(path) > 1 && path[0] == '/'){
        fprintf(stderr, "check_ignore needs relative path not absolute %s\n", path);
        return -1;
    }

    int res = check_ignore_scoped(rules->scoped, path);
    if (res != -1){
        return res;
    }

    return check_ignore_absolute(rules->absolute, rules->absolute_len, path);
}
