#ifndef GIT_CLONE_BRIDGES_H
#define GIT_CLONE_BRIDGES_H

#include "repository.h"
#include "git_object.h"

int cmd_init(char *path);
int cmd_cat_file(char *object, char *fmt);
int cmd_hash_object(char *path, char *type, bool write);
int cmd_log(char *commit);

#endif
