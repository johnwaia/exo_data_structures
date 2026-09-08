#include <stdio.h>
#include <stdlib.h>
#include "dynamic_array.h"

#define INITIAL_CAPACITY 16

static void da_grow_if_full(DynamicArray *arr) {
    if (arr->size == arr->capacity) {
        int new_capacity = (arr->capacity == 0) ? INITIAL_CAPACITY : arr->capacity * 2;
        int *new_data = (int *)realloc(arr->data, (size_t)new_capacity * sizeof(int));
        if (new_data == NULL) {
            fprintf(stderr, "DynamicArray: echec de realloc\n");
            exit(EXIT_FAILURE);
        }
        arr->data = new_data;
        arr->capacity = new_capacity;
    }
}

void da_init(DynamicArray *arr) {
    arr->data = NULL;
    arr->size = 0;
    arr->capacity = 0;
}

/* O(n) : decale tous les elements d'un cran vers la droite. */
void da_insert_front(DynamicArray *arr, int value) {
    int i;
    da_grow_if_full(arr);
    for (i = arr->size; i > 0; i--) {
        arr->data[i] = arr->data[i - 1];
    }
    arr->data[0] = value;
    arr->size++;
}

/* O(1) amorti grace au doublement de capacite. */
void da_insert_back(DynamicArray *arr, int value) {
    da_grow_if_full(arr);
    arr->data[arr->size] = value;
    arr->size++;
}

/* O(n) : parcours lineaire. */
int da_find(const DynamicArray *arr, int value) {
    int i;
    for (i = 0; i < arr->size; i++) {
        if (arr->data[i] == value) {
            return i;
        }
    }
    return -1;
}

/* O(1) : acces direct par index. */
int da_get(const DynamicArray *arr, int index) {
    if (index < 0 || index >= arr->size) {
        fprintf(stderr, "DynamicArray: index hors bornes\n");
        exit(EXIT_FAILURE);
    }
    return arr->data[index];
}

/* O(n) : decale tous les elements restants d'un cran vers la gauche. */
void da_remove_front(DynamicArray *arr) {
    int i;
    if (arr->size == 0) {
        return;
    }
    for (i = 0; i < arr->size - 1; i++) {
        arr->data[i] = arr->data[i + 1];
    }
    arr->size--;
}

void da_free_collection(DynamicArray *arr) {
    free(arr->data);
    arr->data = NULL;
    arr->size = 0;
    arr->capacity = 0;
}
