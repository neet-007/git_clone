#include <stdio.h>
#include <string.h>
#include "bridges.h"

int main(int argc, char *argv[]){
    if (argc < 2){
        fprintf(stderr, "usage:git_clone COMMAND args --flags\n");
        return 1;
    }

    if (strcmp(argv[1], "init") == 0){
        if (argc > 2){
            fprintf(stderr, "can only pass path to init command\n");
            return 1;
        }
        return cmd_init(argv[1]);
    }

    return 0;
}
