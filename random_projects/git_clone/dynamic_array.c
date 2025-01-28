#include "kvlm.h"

DynamicArray *new_dynamic_array(DynamicArrayType type, size_t size){
    if (size == 0){
        size = GROW_CAPACITY(size);
    }

    DynamicArray *array = malloc(sizeof(DynamicArray));
    if (array == NULL){
        fprintf(stderr, "unable to allocate memory for array\n");
        return NULL;
    }

    array->count = 0;
    array->size = size;
    array->type = type;
    switch (type) {
        case TYPE_ARRAY_INT:{
            array->elements = calloc(size, sizeof(int));
            if (array->elements == NULL){
                fprintf(stderr, "unable to allocate memory for array elements\n");
                return NULL;
            }
            break;
        }
        case TYPE_ARRAY_STR:{
            array->elements = calloc(size, sizeof(char *));
            if (array->elements == NULL){
                fprintf(stderr, "unable to allocate memory for array elements\n");
                return NULL;
            }
            break;
        }
        default:{
            fprintf(stderr, "unkown type for array %d\n", type);
            return NULL;
        }
    }

    return array;
}

int reallocate_dynamic_array(DynamicArray *array){
    size_t array_size = array->size;
    array->size = GROW_CAPACITY(array->size);
    switch (array->type) {
        case TYPE_ARRAY_INT:{
            array->elements = GROW_ARRAY(int, array->elements, array_size, array->size);
            break;
        }
        case TYPE_ARRAY_STR:{
            array->elements = GROW_ARRAY(char *, array->elements, array_size, array->size);
            break;
        }
        default:{
            fprintf(stderr, "unkown type for array %d\n", array->type);
            return -1;
        }
    }

    return 1;
}

int add_dynamic_array(DynamicArray *array, void *value){
    switch (array->type) {
        case TYPE_ARRAY_INT:{
            int *elements = array->elements;
            elements[array->count++] = (*(int *)value);
            break;
        }
        case TYPE_ARRAY_STR:{
            char **elements = array->elements;
            char *new = malloc(sizeof(char) * (strlen((char *)value) + 1));
            if (new == NULL){
                fprintf(stderr, "unable to allocate memory for string for array\n");
                return -1;
            }
            new[0] = '\0';
            strcpy(new, (char *)value);
            new[strlen((char *)value)] = '\0';

            elements[array->count++] = new;
            break;
        }
        default:{
            fprintf(stderr, "unkown type for array %d\n", array->type);
            return -1;
        }
    }

    if (array->count >= array->size){
        int res = reallocate_dynamic_array(array);
        if (res < 1){
            fprintf(stderr, "unable to allocate memory for array %d\n", array->type);
            return -1;
        }
    }

    return 1;
}

int remove_dynamic_array(DynamicArray *array, size_t idx){
    if (idx > array->count){
        fprintf(stderr, "idx is greater than array size %ld vs %ld\n", idx, array->size);
        return -1;
    }

    switch (array->type) {
        case TYPE_ARRAY_INT:{
            int *elements = array->elements;
            for (size_t i = idx; i < array->count - 1; i++){
                elements[i] = elements[i + 1];
            }
            break;
        }
        case TYPE_ARRAY_STR:{
            char **elements = array->elements;
            for (size_t i = idx; i < array->count - 1; i++){
                elements[i] = elements[i + 1];
            }
            break;
        }
        default:{
            fprintf(stderr, "unkown type for array %d\n", array->type);
            return -1;
        }
    }

    return 1;
}

int remove_dynamic_array_by_value(DynamicArray *array, void *value){
    size_t idx;
    size_t i;
    switch (array->type) {
        case TYPE_ARRAY_INT:{
            int *elements = array->elements;
            for (i = 0; i < array->count; i++){
                if (elements[i] == (*(int *)value)){
                    idx = i;
                }
            }
            if (i == array->count){
                fprintf(stderr, "could not find value in array %d\n", (* (int *)value));
                return -1;
            }

            for (i = idx; i < array->count - 1; i++){
                elements[i] = elements[i + 1];
            }
            break;
        }
        case TYPE_ARRAY_STR:{
            char **elements = array->elements;
            for (i = 0; i < array->count; i++){
                if (strcmp(elements[i], (char *)value) == 0){
                    idx = i;
                }
            }
            if (i == array->count){
                fprintf(stderr, "could not find value in array %d\n", (* (int *)value));
                return -1;
            }

            for (i = idx; i < array->count - 1; i++){
                elements[i] = elements[i + 1];
            }
            break;
        }
        default:{
            fprintf(stderr, "unkown type for array %d\n", array->type);
            return -1;
        }
    }

    return 1;
}

int index_of_dynamic_array(DynamicArray *array, void *value){
    size_t idx;
    size_t i;
    switch (array->type) {
        case TYPE_ARRAY_INT:{
            int *elements = array->elements;
            for (i = 0; i < array->count; i++){
                if (elements[i] == (*(int *)value)){
                    idx = i;
                }
            }
            if (i == array->count){
                return -1;
            }

            break;
        }
        case TYPE_ARRAY_STR:{
            char **elements = array->elements;
            for (i = 0; i < array->count; i++){
                if (strcmp(elements[i], (char *)value) == 0){
                    idx = i;
                }
            }
            if (i == array->count){
                return -1;
            }

            break;
        }
        default:{
            fprintf(stderr, "unkown type for array %d\n", array->type);
            return -1;
        }
    }

    return idx;
}

void print_dynamic_array(DynamicArray *array){
    printf("\nDynamic array\n-------------------\n");

    switch (array->type) {
        case TYPE_ARRAY_INT:{
            int *elements = array->elements;
            for (size_t i = 0; i < array->count; i++){
                printf("%d, ", elements[i]);
            }
            break;
        }
        case TYPE_ARRAY_STR:{
            char **elements = array->elements;
            for (size_t i = 0; i < array->count; i++){
                printf("%s, ", elements[i]);
            }
            break;
        }
        default:{
            fprintf(stderr, "unkown type for array %d\n", array->type);
            return;
        }
    }

    printf("\n");
}

int free_dynamic_array(DynamicArray *array){
    switch (array->type) {
        case TYPE_ARRAY_INT:{
            array->elements = FREE_ARRAY(int, array->elements, array->size);
            break;
        }
        case TYPE_ARRAY_STR:{
            char **elements = array->elements;
            for (size_t i = 0; i < array->count; i++){
                free(elements[i]);
            }
            array->elements = FREE_ARRAY(char *, array->elements, array->size);
            break;
        }
        default:{
            fprintf(stderr, "unkown type for array %d\n", array->type);
            return -1;
        }
    }

    free(array);
    return 1;
}

void *reallocate(void *pointer, size_t old_size, size_t new_size){
    if (new_size == 0){
        free(pointer);
        return NULL;
    }

    pointer = realloc(pointer, new_size);
    if (pointer == NULL){
        fprintf(stderr, "unable to allocate memory for array\n");
        return NULL;
    }

    return pointer;
}
