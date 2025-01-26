#ifndef GIT_CLONE_INI_PARSER_H
#define GIT_CLONE_INI_PARSER_H

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <dirent.h>
#include "hash_table.h"
#include "utils.h"

typedef struct Ini{
    char *filename;
    HashTable *sections;
}Ini;

Ini *ini_new(char *filename);
Ini *ini_parse(const char *filename);
int parse_key_val(FILE *file, int *key_len, int *val_len, char **key, char **val);
char *parse_section(FILE *file, int *len);
char *parse_comment(FILE *file, int *len);
int write_ini_section(HashTable *section, FILE *f);
int ini_write(Ini *ini, FILE *f);
int ini_add_section(Ini *ini, char *section_name);
Ht_item *ini_get_key(Ini *ini, char *section_name, char *key);
int ini_set_key(Ini *ini, char *section_name, char *key, char *value);
void free_ini(Ini *ini);

#endif
