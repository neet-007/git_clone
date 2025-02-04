#include "refs.h"

//WARNING: review this and see how it fits with other commands
char *ref_resolver(GitRepository *repo, char *ref){
    char *path = repo_file(repo, false, 1, ref);
    if (path == NULL){
        fprintf(stderr, "unalbe to file ref %s in ref_resolver\n", ref);
        return NULL;
    }
    /*
    char *path = ref;
    */

    if (!is_file(path)){
        //free(path);
        return NULL;
    }

    size_t ref_str_size = 0;
    size_t ref_str_curr = 0;
    char *ref_str = NULL;
    FILE *f = fopen(path, "r");
    if (f == NULL){
        fprintf(stderr, "unalbe to open ref file ref %s file %s in ref_resolver\n", ref, path);
        //free(path);
        return NULL;
    }

    while(fgetc(f) != EOF){
        ref_str_size++;
    }

    fseek(f, -((int)ref_str_size), SEEK_END);
    ref_str = malloc(sizeof(char) * ref_str_size);
    if (ref_str == NULL){
        fprintf(stderr, "unalbe to allocate memort for ref %s file %s in ref_resolver\n", ref, path);
        fclose(f);
        //free(path);
        return NULL;
    }
    free(path);

    while(ref_str_curr < ref_str_size - 1){
        ref_str[ref_str_curr++] = fgetc(f);
    }
    ref_str[ref_str_curr] = '\0';
    fclose(f);

    if (strncmp(ref_str, "ref: ", strlen("ref: ")) == 0){
        char * temp = ref_resolver(repo, ref_str + 5);
        free(ref_str);
        if (temp == NULL){
            fprintf(stderr, "unable to ref_resolve in ref_reslover\n");
            return NULL;
        }
        ref_str = temp;
    }

    return ref_str;
}

// path=NULL
HashTable *ref_list(GitRepository *repo, char *path){
#define FREE_DIRS()\
    size_t j = 0;\
    while (j < dirs_count){\
        if (dirs[j]){\
            free(dirs[j]);\
        }\
        j++;\
    }\
    free(dirs);\

    bool free_path = false;
    if (path == NULL){
        free_path = true;
        path = repo_dir(repo, false, 1, "refs");
        if (path == NULL){
            fprintf(stderr, "unable to find refs in repo\n");
            return NULL;
        }
    }

    size_t i = 0;
    size_t dirs_count = 0;
    char *curr_ref = NULL;
    char *curr = NULL;
    char *can = NULL;
    char **dirs = list_directory(path, &dirs_count, true);
    if (dirs == NULL){
        fprintf(stderr, "unable to list dirs for path %s in ref_list\n", path);
        if (free_path){
            free(path);
        }
        return NULL;
    }
    HashTable *curr_ref_ht = NULL;
    HashTable *ret = create_table(CAPACITY);
    if (ret == NULL){
        fprintf(stderr, "unable to craete table in ref_list\n");
        if (free_path){
            free(path);
        }
        FREE_DIRS();
        return NULL;
    }

    while(i < dirs_count){
        curr = dirs[i];
        can = join_path(path, 1, curr);
        if (can == NULL){
            fprintf(stderr, "unable to join paths path %s path %s in ref_list\n", path, curr);
            if (free_path){
                free(path);
            }
            free_table(ret);
            FREE_DIRS();
            return NULL;
        }

        if (is_dir(can)){
            curr_ref_ht = ref_list(repo, can);
            if (curr_ref_ht == NULL){
                fprintf(stderr, "unable to ref_list in ref_list\n");
                if (free_path){
                    free(path);
                }
                free_table(ret);
                FREE_DIRS();
                return NULL;
            }
            ht_insert(ret, curr, curr_ref_ht, sizeof(HashTable), TYPE_HASH_TABLE);
        }else{
            curr_ref = ref_resolver(repo, can);
            if (curr_ref == NULL){
                i++;
                continue;
            }
            ht_insert(ret, curr, curr_ref, strlen(curr_ref) + 1, TYPE_STR);
            free(curr_ref);
        }
        free(can);
        i++;
    }

    if (free_path){
        free(path);
    }
    FREE_DIRS();
#undef FREE_DIRS

    return ret;
}
