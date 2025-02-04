#include <stdio.h>
#include <string.h>
#include "bridges.h"
#include "git_object_types.h"

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
            GitObjectType type;
            if (strcmp(argv[2], "blob") == 0){
                type = TYPE_BLOB;
            }else if (strcmp(argv[2], "commit") == 0){
                type = TYPE_COMMIT;
            }else if (strcmp(argv[2], "tag") == 0){
                type = TYPE_TAG;
            }else if (strcmp(argv[2], "tree") == 0){
                type = TYPE_TREE;
            }else{
                fprintf(stderr, "usage:git_clone cat-file TYPE OBJECT\n");
                return 1;
            }
            return cmd_cat_file(argv[3], type);
        }
        if (argc == 3){
            return cmd_cat_file(argv[2], TYPE_NONE);
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
    if (strcmp(argv[1], "show-ref") == 0){
        if (argc != 2){
            fprintf(stderr, "usage:git_clone show-ref\n");
            return 1;
        }

        return cmd_show_ref();
    }
    if (strcmp(argv[1], "tag") == 0){
        if (argc == 2){
            return cmd_git_tag(NULL, NULL, false);
        }
        if (2 < argc && argc < 6){
            bool a = false;
            char *name = NULL;
            char *object = NULL;

            for (int i = 2; i < argc; i++){
                if (argv[i][0] == '-'){
                    if (strlen(argv[i]) == 2 && argv[i][1] == 'a'){
                        a = true;
                        continue;
                    }

                    fprintf(stderr, "usage:git_clone tag [-a] NAME OBJECT\n");
                    return 1;
                }
                if (name == NULL){
                    name = argv[i];
                }else if(object == NULL){
                    object = argv[i];
                }else{
                    fprintf(stderr, "usage:git_clone tag [-a] NAME OBJECT\n");
                    return 1;
                }
            }

            return cmd_git_tag(name, object, a);
        }

        fprintf(stderr, "usage:git_clone tag [-a] NAME OBJECT\n");
        return 1;
    }
    if (strcmp(argv[1], "rev-parse") == 0){
        GitObjectType type = TYPE_NONE;
        if (argc == 3){
            return cmd_rev_parse(argv[1], type);
        }
        if (argc == 5){
            char *name = NULL;
            for (int i = 2; i < argc; i++){
                if (strlen(argv[i]) == 16 && argv[i][0] == '-' && argv[i][1] == '-'){
                    if (strcmp(argv[i], "--git-clone-type") == 0){
                        if (i + 1 < argc){
                            if (strcmp(argv[i + 1], "blob") == 0){
                                type = TYPE_BLOB;
                            }else if (strcmp(argv[i + 1], "commit") == 0){
                                type = TYPE_COMMIT;
                            }else if (strcmp(argv[i + 1], "tag") == 0){
                                type = TYPE_TAG;
                            }else if (strcmp(argv[i + 1], "tree") == 0){
                                type = TYPE_TREE;
                            }else{
                                fprintf(stderr, "usage:git_clone rev-parse [--git-clone-type TYPE] NAME\n");
                                return 1;
                            }
                            i++;
                            continue;
                        }
                    }
                    fprintf(stderr, "usage:git_clone rev-parse [--git-clone-type TYPE] NAME\n");
                    return 1;
                }
                name = argv[i];
            }
            return cmd_rev_parse(name, type);
        }

        fprintf(stderr, "usage:git_clone rev-parse [--git-clone-type TYPE] NAME\n");
        return 1;
    }
    if (strcmp(argv[1], "ls-files") == 0){
        if (argc == 2){
            return cmd_ls_files(false);
        }
        if (argc == 3){
            if (strcmp(argv[2], "--verbose") == 0){
                return cmd_ls_files(true);
            }
        }

        fprintf(stderr, "usage:git_clone ls-files [--verbose]\n");
        return 1;
    }
    if (strcmp(argv[1], "check-ignore") == 0){
        if (argc < 3){
            fprintf(stderr, "usage:git_clone check-ignore PATHS...\n");
            return 1;
        }

        return cmd_check_ignore(argc - 2, argv + 2);
    }
    if (strcmp(argv[1], "status") == 0){
        if (argc != 2){
            fprintf(stderr, "usage:git_clone status\n");
            return 1;
        }

        return cmd_status();
    }

    return 0;
}
