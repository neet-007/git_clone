#ifndef GIT_CLONE_BRIDGES_H
#define GIT_CLONE_BRIDGES_H

#include <time.h>
#include <pwd.h>
#include <grp.h>
#include <unistd.h>
#include "repository.h"
#include "git_object.h"
#include "git_object_types.h"
#include "hash_table.h"
#include "utils.h"
#include "refs.h"
#include "git_index.h"
#include "git_ignore.h"

int cmd_init(char *path);
int cat_file(GitRepository *repo, char *object, GitObjectType fmt);
int cmd_cat_file(char *object, GitObjectType fmt);
int cmd_hash_object(char *path, char *type, bool write);
int cmd_log(char *commit);
int cmd_ls_tree(char *tree, bool r);
int cmd_checkout(char *commit, char *path);
int cmd_show_ref();
int cmd_git_tag(char *name, char *object, bool a);
int cmd_rev_parse(char *name, GitObjectType type);
int cmd_ls_files(bool verbose);
int cmd_check_ignore(size_t count, char **paths);
int cmd_status();

#endif
