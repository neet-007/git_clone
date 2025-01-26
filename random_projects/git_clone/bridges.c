#include "bridges.h"

int cmd_init(char *path){
    if (repo_create(path) == NULL){
        return 1;
    }

    return 0;
}
