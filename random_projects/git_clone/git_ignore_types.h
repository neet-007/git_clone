#include <stddef.h>

typedef struct GitIgnoreItem{
    bool included;
    char *path;
} GitIgnoreItem;

typedef struct GitIgnoreItems{
    size_t size;
    size_t len;
    GitIgnoreItem **items;
} GitIgnoreItems;

