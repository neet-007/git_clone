#ifndef GIT_CLONE_UTILS_H
#define GIT_CLONE_UTILS_H

#define PATH_EXISTS(path) ((path) != NULL && access((path), F_OK) != -1)
#define CHUNK 16384

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <dirent.h>
#include <sys/stat.h>
#include <ctype.h> 
#include <zlib.h>
#include <openssl/sha.h> 

char *join_path_(char *path, unsigned int count, va_list va_a);
char *join_path(char *path, unsigned int count, ...);
int make_directories(char *path);
bool is_dir(char *path);
bool is_file(char *path);
int is_dir_empty(char *path);
int count_digits(int num);
char* compress_zlib(char *input, size_t input_len, size_t *output_size);
unsigned char *decompress_zlib_from_data(const unsigned char *src, size_t src_len, size_t *dst_len);
unsigned char *decompress_zlib_from_file(FILE *file, size_t *dst_len);
void print_raw_data_as_chars(char *data, size_t size);
void sha1_hexdigest(unsigned char *data, size_t len, char *output);
char *file_read_all(FILE *f, size_t *data_size);
int find_char(char *raw, char c, int start, size_t raw_size);
char **list_directory(char *path, size_t *count, bool sort);
bool str_empty(char *str);
void to_lower(char *str);
bool ends_with(char *str, char *suffix);
void dirname(char **path);
char *dirname_copy(char *path);
char *read_line_from_raw(char **dest, char *raw, size_t *dest_size, size_t *start, size_t raw_size);

#endif
