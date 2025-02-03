#ifndef GIT_CLONE_DYNAMIC_ARRAY
#define GIT_CLONE_DYNAMIC_ARRAY

#define GROW_CAPACITY(capacity) \
    ((capacity) < 8 ? 8 : (capacity) * 2)

#define GROW_ARRAY(type, pointer, old_count, new_count)\
    (type *)reallocate(pointer, sizeof(type) * (old_count), sizeof(type) * (new_count))

#define FREE_ARRAY(type, pointer, old_count)\
    reallocate(pointer, sizeof(type) * (old_count), 0)

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef enum DynamicArrayType{
    TYPE_ARRAY_INT,
    TYPE_ARRAY_STR,
} DynamicArrayType;

typedef struct DynamicArray{
    DynamicArrayType type;
    size_t count;
    size_t size;
    void *elements;
} DynamicArray;

DynamicArray *new_dynamic_array(DynamicArrayType type, size_t size);
int reallocate_dynamic_array(DynamicArray *array);
int add_dynamic_array(DynamicArray *array, void *value);
int remove_dynamic_array(DynamicArray *array, size_t idx);
int remove_dynamic_array_by_value(DynamicArray *array, void *value);
int index_of_dynamic_array(DynamicArray *array, void *value);
void print_dynamic_array(DynamicArray *array);
int free_dynamic_array(DynamicArray *array);
void *reallocate(void *pointer, size_t old_size, size_t new_size);

#endif
