#include <stdio.h>
#include <string.h>
#include "bridges.h"

int main(int argc, char *argv[]){
    if (argc < 2){
        fprintf(stderr, "usage:git_clone COMMAND args --flags\n");
        return 1;
    }

    if (strcmp(argv[1], "init") == 0){
        if (argc > 3){
            fprintf(stderr, "usage:git_clone init DIR\n");
            return 1;
        }
        return cmd_init(argv[2]);
    }
    if (strcmp(argv[1], "cat-file") == 0){
        if (argc == 4){
            return cmd_cat_file(argv[3], argv[2]);
        }
        if (argc == 3){
            return cmd_cat_file(argv[2], "");
        }
        fprintf(stderr, "usage:git_clone cat-file TYPE OBJECT\n");
        return 1;
    }
    if (strcmp(argv[1], "hash-object") == 0){
        if (argc > 6){
            fprintf(stderr, "1usage:git_clone hash-object [-w] [-t TYPE] FILE\n");
            return 1;
        }

        bool write = false;
        char *type = NULL;
        char *path = NULL;
        size_t curr_len = 0;

        for (int i = 2; i < argc; i++){
            curr_len = strlen(argv[i]);
            if (curr_len > 1 && argv[i][0] == '-'){
                if (argv[i][1] == 'w'){
                    write = true;
                    continue;
                }
                if (argv[i][1] == 't'){
                    if (i + 1 >= argc || strlen(argv[i + 1]) < 4){
                        fprintf(stderr, "2usage:git_clone hash-object [-w] [-t TYPE] FILE\n");
                        return 1;
                    }
                    type = argv[i + 1];
                    continue;
                }
                fprintf(stderr, "3usage:git_clone hash-object [-w] [-t TYPE] FILE\n");
                return 1;
            }
            path = argv[i];
        }

        if (type == NULL || path == NULL){
            fprintf(stderr, "4usage:git_clone hash-object [-w] [-t TYPE] FILE\n");
            return 1;
        }

        return cmd_hash_object(path, type, write);
    }
    if (strcmp(argv[1], "log") == 0){
        if (argc == 2){
            return cmd_log("HEAD");
        }
        if (argc == 3){
            return cmd_log(argv[2]);
        }

        fprintf(stderr, "usage:git_clone log COMMIT\n");
        return 1;
    }
    if (strcmp(argv[1], "ls-tree") == 0){
        char *tree = NULL;
        bool r = false;

        for (int i = 2; i < argc; i++){
            if (argv[i][0] == '-'){
                if (strlen(argv[i]) == 2 && argv[i][1] == 'r'){
                    r = true;
                    continue;
                }

                fprintf(stderr, "usage:git_clone ls-tree [-r] TREE");
                return 1;
            }
            tree = argv[i];
        }

        return cmd_ls_tree(tree, r);
    }
    if (strcmp(argv[1], "checkout") == 0){
        if (argc != 4){
            fprintf(stderr, "usage:git_clone checkout COMMIT PATH");
            return 1;
        }

        return cmd_checkout(argv[2], argv[3]);
    }

    return 0;
}
