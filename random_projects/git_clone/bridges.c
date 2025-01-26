#include "bridges.h"

int cmd_init(char *path){
    if (repo_create(path) == NULL){
        fprintf(stderr, "faild to ini repo\n");
        return 1;
    }

    return 0;
}
